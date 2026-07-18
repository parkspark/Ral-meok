#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <math.h>

#define W 800
#define H 600
#define MAX_E 220
#define MAX_P 160
#define MAX_EP 64
#define MAX_POP 36
#define PI 3.14159265f

enum { NORMAL, ELITE, BOSS };
enum { MENU_NONE, MENU_LEVEL, MENU_RELIC, MENU_SHOP };

typedef struct { float x,y,hp,maxhp,spd,hit_cd,shoot_cd; unsigned char live,type; } Enemy;
typedef struct { float x,y,vx,vy,life; int dmg; unsigned char live,pierce; } Shot;
typedef struct { float x,y,vx,vy,life; unsigned char live; } EnemyShot;
typedef struct { float x,y,t; COLORREF c; unsigned char on; } Pop;

static HWND wnd;
static HDC memdc;
static HBITMAP dib,oldbmp;
static uint32_t *pix;
static Enemy e[MAX_E];
static Shot p[MAX_P];
static EnemyShot ep[MAX_EP];
static Pop pops[MAX_POP];
static unsigned char keys[256],hits[256];
static float px,py,hp,maxhp,move_spd,base_dmg,attack_cd;
static float invuln,fire_cd,spawn_cd,time_alive,shake,nova_cd,nova_fx;
static float next_elite,next_boss,boss_warning;
static int pending_boss,kills,game_over,level,xp,xp_need,coins;
static int bolt_level,orbit_level,nova_level,menu,choice[3],relic[6];
static int bosses,elites,shop_buys,spawn_tick;
static uint32_t rng=0x31415926u;
static LARGE_INTEGER freq,last;

static uint32_t rndu(void){ rng=rng*1664525u+1013904223u; return rng; }
static float rndf(float a,float b){ return a+(float)(rndu()&65535)/65535.0f*(b-a); }
static float invsqrt(float x){ float y=x>1.0f?x:1.0f; int i; if(x<=.0001f)return 0; for(i=0;i<12;i++)y=.5f*(y+x/y); return 1.0f/y; }
static float clampf(float x,float a,float b){ return x<a?a:(x>b?b:x); }
static int eradius(int type){ return type==BOSS?34:(type==ELITE?20:13); }

static void addpop(float x,float y,COLORREF c){
    int i; for(i=0;i<MAX_POP;i++)if(!pops[i].on){pops[i].on=1;pops[i].x=x;pops[i].y=y;pops[i].t=.28f;pops[i].c=c;return;}
}

static void make_choices(int count){
    int i,j,v; for(i=0;i<3;i++){ do{v=(int)(rndu()%count); for(j=0;j<i&&choice[j]!=v;j++); }while(j<i); choice[i]=v; }
}

static void make_level_choices(void){
    choice[0]=(int)(rndu()%3);
    choice[1]=3+(int)(rndu()%3);
    do{choice[2]=(int)(rndu()%6);}while(choice[2]==choice[0]||choice[2]==choice[1]);
}

static void reset_game(void){
    ZeroMemory(e,sizeof(e)); ZeroMemory(p,sizeof(p)); ZeroMemory(ep,sizeof(ep)); ZeroMemory(pops,sizeof(pops));
    ZeroMemory(keys,sizeof(keys)); ZeroMemory(hits,sizeof(hits)); ZeroMemory(relic,sizeof(relic));
    px=W*.5f;py=H*.5f;maxhp=100;hp=maxhp;move_spd=190;base_dmg=15;attack_cd=.52f;
    invuln=fire_cd=shake=nova_fx=0;spawn_cd=.25f;time_alive=0;nova_cd=7;
    next_elite=18;next_boss=60;boss_warning=0;pending_boss=0;kills=0;game_over=0;
    level=1;xp=0;xp_need=12;coins=0;bolt_level=1;orbit_level=0;nova_level=0;
    menu=MENU_NONE;bosses=elites=shop_buys=spawn_tick=0;
}

static void spawn_enemy_type(int type){
    int i,side;float x=0,y=0,t=time_alive;
    for(i=0;i<MAX_E;i++)if(!e[i].live){
        side=(int)(rndu()%4);
        if(side==0){x=rndf(20,W-20);y=-40;}else if(side==1){x=W+40;y=rndf(20,H-20);}
        else if(side==2){x=rndf(20,W-20);y=H+40;}else{x=-40;y=rndf(20,H-20);}
        e[i].live=1;e[i].type=(unsigned char)type;e[i].x=x;e[i].y=y;e[i].hit_cd=0;
        if(type==NORMAL){e[i].maxhp=18+t*.10f;e[i].spd=42+rndf(0,16);}
        else if(type==ELITE){e[i].maxhp=105+t*.65f;e[i].spd=52+rndf(0,10);elites++;}
        else{e[i].maxhp=320+bosses*180+t*.7f;e[i].spd=27+bosses*2;e[i].shoot_cd=1.8f;}
        e[i].hp=e[i].maxhp;return;
    }
}

static void open_level(void){ menu=MENU_LEVEL;make_level_choices(); }
static void check_level(void){
    if(menu==MENU_NONE&&xp>=xp_need){xp-=xp_need;level++;xp_need=10+level*8;open_level();}
}

static void award_kill(int type,float x,float y){
    int gain=type==BOSS?30:(type==ELITE?9:2);
    kills++;xp+=gain;coins+=(type==BOSS?22:(type==ELITE?6:1))+relic[5];
    if(relic[3]&&kills%(12-relic[3]*2)==0){hp=clampf(hp+5*relic[3],0,maxhp);addpop(px,py,RGB(255,80,150));}
    addpop(x,y,type==BOSS?RGB(255,210,70):RGB(100,255,150));
    if(type==BOSS){bosses++;menu=MENU_RELIC;make_choices(6);}
}

static void damage_enemy(int i,int dmg){
    int type;if(!e[i].live)return;e[i].hp-=dmg;addpop(e[i].x,e[i].y,RGB(255,150,70));
    if(e[i].hp<=0){type=e[i].type;e[i].live=0;award_kill(type,e[i].x,e[i].y);}
}

static void fire_shot(void){
    int i,best=-1;float bd=99999999,dx,dy,d2,iv;
    for(i=0;i<MAX_E;i++)if(e[i].live){dx=e[i].x-px;dy=e[i].y-py;d2=dx*dx+dy*dy;if(d2<bd){bd=d2;best=i;}}
    if(best<0)return;
    for(i=0;i<MAX_P;i++)if(!p[i].live){
        dx=e[best].x-px;dy=e[best].y-py;iv=invsqrt(dx*dx+dy*dy);
        p[i].live=1;p[i].x=px;p[i].y=py;p[i].vx=dx*iv*460;p[i].vy=dy*iv*460;
        p[i].life=1.7f;p[i].dmg=(int)(base_dmg+bolt_level*4);p[i].pierce=(unsigned char)(bolt_level>=4?1:0);
        addpop(px,py,RGB(255,245,90));return;
    }
}

static void boss_fire(int idx){
    int i;float dx=px-e[idx].x,dy=py-e[idx].y,iv=invsqrt(dx*dx+dy*dy);
    for(i=0;i<MAX_EP;i++)if(!ep[i].live){ep[i].live=1;ep[i].x=e[idx].x;ep[i].y=e[idx].y;ep[i].vx=dx*iv*115;ep[i].vy=dy*iv*115;ep[i].life=6;return;}
}

static void hurt_player(float dmg){
    int i;if(invuln>0||game_over)return;hp-=dmg;invuln=.55f;shake=.20f;addpop(px,py,RGB(255,70,70));
    if(relic[4])for(i=0;i<MAX_E;i++)if(e[i].live){float dx=e[i].x-px,dy=e[i].y-py;if(dx*dx+dy*dy<70*70)damage_enemy(i,18*relic[4]);}
    if(hp<=0){hp=0;game_over=1;menu=MENU_NONE;}
}

static void apply_skill(int id){
    if(id==0)bolt_level++;
    else if(id==1)orbit_level++;
    else if(id==2)nova_level++;
    else if(id==3)base_dmg+=5;
    else if(id==4)move_spd+=18;
    else{maxhp+=20;hp=clampf(hp+20,0,maxhp);}
    menu=MENU_NONE;addpop(px,py,RGB(80,220,255));
}

static void apply_relic(int id){
    relic[id]++;if(relic[id]>3)relic[id]=3;
    if(id==0){maxhp+=30;hp=clampf(hp+30,0,maxhp);}
    else if(id==1){attack_cd*=.87f;if(attack_cd<.22f)attack_cd=.22f;}
    else if(id==2)move_spd+=22;
    menu=MENU_NONE;addpop(px,py,RGB(255,210,65));
}

static void update_menu(void){
    int pick=-1;if(hits['1'])pick=0;else if(hits['2'])pick=1;else if(hits['3'])pick=2;
    if(menu==MENU_LEVEL&&pick>=0)apply_skill(choice[pick]);
    else if(menu==MENU_RELIC&&pick>=0)apply_relic(choice[pick]);
    else if(menu==MENU_SHOP){
        if((hits['B']||hits['4']||hits[VK_ESCAPE]))menu=MENU_NONE;
        else if(pick==0&&coins>=12&&hp<maxhp){coins-=12;hp=clampf(hp+40,0,maxhp);shop_buys++;addpop(px,py,RGB(90,255,130));}
        else if(pick==1&&coins>=20){coins-=20;base_dmg+=4;shop_buys++;addpop(px,py,RGB(255,180,70));}
        else if(pick==2&&coins>=24){coins-=24;maxhp+=20;hp+=20;shop_buys++;addpop(px,py,RGB(90,210,255));}
    }
}

static void update_game(float dt){
    int i,j,stage;float mx=0,my=0,iv,dx,dy,d2,interval,ang,ox,oy,rad;
    if(game_over){if(hits[VK_SPACE])reset_game();return;}
    if(menu!=MENU_NONE){update_menu();return;}
    if(hits['B']){menu=MENU_SHOP;return;}

    time_alive+=dt;if(invuln>0)invuln-=dt;if(fire_cd>0)fire_cd-=dt;if(shake>0)shake-=dt;if(nova_fx>0)nova_fx-=dt;
    if(keys['A']||keys[VK_LEFT])mx-=1;
    if(keys['D']||keys[VK_RIGHT])mx+=1;
    if(keys['W']||keys[VK_UP])my-=1;
    if(keys['S']||keys[VK_DOWN])my+=1;
    if(mx&&my){mx*=.7071067f;my*=.7071067f;}px+=mx*move_spd*dt;py+=my*move_spd*dt;
    px=clampf(px,14,W-14);py=clampf(py,66,H-28);

    stage=time_alive>=60?2:(time_alive>=30?1:0);interval=stage==0?1.5f:(stage==1?1.0f:.6f);
    spawn_cd-=dt;while(spawn_cd<=0){spawn_enemy_type(NORMAL);spawn_cd+=interval;if(stage==2&&(spawn_tick++&1))spawn_enemy_type(NORMAL);}
    if(time_alive>=next_elite){spawn_enemy_type(ELITE);next_elite+=18;}
    if(time_alive>=next_boss&&!pending_boss){pending_boss=1;boss_warning=2.5f;next_boss+=60;}
    if(pending_boss){boss_warning-=dt;if(boss_warning<=0){pending_boss=0;spawn_enemy_type(BOSS);}}

    for(i=0;i<MAX_E;i++)if(e[i].live){
        if(e[i].hit_cd>0)e[i].hit_cd-=dt;
        dx=px-e[i].x;dy=py-e[i].y;iv=invsqrt(dx*dx+dy*dy);
        e[i].x+=dx*iv*e[i].spd*(stage==1?1.1f:(stage==2?1.2f:1))*dt;e[i].y+=dy*iv*e[i].spd*(stage==1?1.1f:(stage==2?1.2f:1))*dt;
        if(e[i].type==BOSS){e[i].shoot_cd-=dt;if(e[i].shoot_cd<=0){boss_fire(i);e[i].shoot_cd=1.65f-clampf(bosses*.08f,0,.55f);}}
        d2=(e[i].x-px)*(e[i].x-px)+(e[i].y-py)*(e[i].y-py);rad=(float)(eradius(e[i].type)+14);
        if(d2<rad*rad)hurt_player(e[i].type==BOSS?14:(e[i].type==ELITE?14:10));
    }

    if(fire_cd<=0){fire_shot();fire_cd=attack_cd-bolt_level*.035f;if(fire_cd<.14f)fire_cd=.14f;}
    for(i=0;i<MAX_P;i++)if(p[i].live){
        p[i].x+=p[i].vx*dt;p[i].y+=p[i].vy*dt;p[i].life-=dt;
        if(p[i].life<=0||p[i].x<-30||p[i].x>W+30||p[i].y<-30||p[i].y>H+30)p[i].live=0;
        if(!p[i].live)continue;
        for(j=0;j<MAX_E;j++)if(e[j].live){dx=e[j].x-p[i].x;dy=e[j].y-p[i].y;rad=(float)(eradius(e[j].type)+5);if(dx*dx+dy*dy<rad*rad){damage_enemy(j,p[i].dmg);if(p[i].pierce)p[i].pierce--;else p[i].live=0;break;}}
    }
    for(i=0;i<MAX_EP;i++)if(ep[i].live){ep[i].x+=ep[i].vx*dt;ep[i].y+=ep[i].vy*dt;ep[i].life-=dt;if(ep[i].life<=0)ep[i].live=0;dx=ep[i].x-px;dy=ep[i].y-py;if(ep[i].live&&dx*dx+dy*dy<18*18){ep[i].live=0;hurt_player(10);}}

    if(orbit_level){
        for(i=0;i<orbit_level+1&&i<6;i++){ang=time_alive*(2.2f+orbit_level*.12f)+i*2*PI/(orbit_level+1);ox=px+(float)(50+orbit_level*3)*cosf(ang);oy=py+(float)(50+orbit_level*3)*sinf(ang);
            for(j=0;j<MAX_E;j++)if(e[j].live&&e[j].hit_cd<=0){dx=e[j].x-ox;dy=e[j].y-oy;rad=(float)(eradius(e[j].type)+7);if(dx*dx+dy*dy<rad*rad){e[j].hit_cd=.32f;damage_enemy(j,7+orbit_level*5);}}
        }
    }
    if(nova_level){nova_cd-=dt;if(nova_cd<=0){rad=125+nova_level*16;for(i=0;i<MAX_E;i++)if(e[i].live){dx=e[i].x-px;dy=e[i].y-py;if(dx*dx+dy*dy<rad*rad)damage_enemy(i,18+nova_level*10);}nova_fx=.35f;nova_cd=9.5f-nova_level*.8f;if(nova_cd<4.5f)nova_cd=4.5f;shake=.1f;}}
    for(i=0;i<MAX_POP;i++)if(pops[i].on){pops[i].t-=dt;if(pops[i].t<=0)pops[i].on=0;}
    check_level();
}

static void fill_rect(HDC dc,int x,int y,int w,int h,COLORREF c){RECT r={x,y,x+w,y+h};SetDCBrushColor(dc,c);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));}
static void ellipse_c(HDC dc,int x,int y,int r,COLORREF c){SetDCBrushColor(dc,c);SelectObject(dc,GetStockObject(DC_BRUSH));Ellipse(dc,x-r,y-r,x+r,y+r);}
static void ring_c(HDC dc,int x,int y,int r,COLORREF c){HGDIOBJ op=SelectObject(dc,GetStockObject(DC_PEN)),ob=SelectObject(dc,GetStockObject(NULL_BRUSH));SetDCPenColor(dc,c);Ellipse(dc,x-r,y-r,x+r,y+r);SelectObject(dc,op);SelectObject(dc,ob);}
static void text(HDC dc,int x,int y,const char*s,COLORREF c){SetTextColor(dc,c);SetBkMode(dc,TRANSPARENT);TextOutA(dc,x,y,s,lstrlenA(s));}

static const char* skill_name(int id){static const char*n[]={"ARC BOLT","ORBIT BLADES","NOVA PULSE","RAW POWER","FLEET STEP","FORTIFY"};return n[id];}
static const char* skill_desc(int id){static const char*n[]={"boost auto shot","circling damage","periodic area blast","+5 weapon damage","+18 move speed","+20 HP and heal"};return n[id];}
static const char* relic_name(int id){static const char*n[]={"VITAL CORE","CHRONO LENS","WIND BOOTS","BLOODSTONE","THORN CROWN","LUCKY COIN"};return n[id];}
static const char* relic_desc(int id){static const char*n[]={"+30 HP and heal","13% faster shots","+22 move speed","heal on streaks","contact retaliation","extra kill coins"};return n[id];}
static int relic_count(void){int i,n=0;for(i=0;i<6;i++)n+=relic[i];return n;}

static void draw_card(int idx,int id,int relic_mode){
    int x=104+idx*204;char b[32];fill_rect(memdc,x,252,184,122,RGB(32,38,52));fill_rect(memdc,x,252,184,4,relic_mode?RGB(255,195,60):RGB(70,190,255));
    wsprintfA(b,"[%d]",idx+1);text(memdc,x+12,266,b,RGB(255,255,255));text(memdc,x+12,292,relic_mode?relic_name(id):skill_name(id),relic_mode?RGB(255,215,90):RGB(100,210,255));
    text(memdc,x+12,324,relic_mode?relic_desc(id):skill_desc(id),RGB(190,198,214));
}

static void render(HDC dc){
    int i,ox=0,oy=0,bar,r;char buf[128];float ang,bx,by;HPEN oldpen=(HPEN)SelectObject(memdc,GetStockObject(NULL_PEN));
    if(shake>0){ox=(int)rndf(-3,3);oy=(int)rndf(-3,3);}fill_rect(memdc,0,0,W,H,RGB(9,12,19));
    for(i=0;i<W;i+=40)fill_rect(memdc,i+ox,60,1,H-84,RGB(22,27,38));
    for(i=60;i<H;i+=40)fill_rect(memdc,0,i+oy,W,1,RGB(22,27,38));
    fill_rect(memdc,0,0,W,60,RGB(15,19,29));fill_rect(memdc,10,10,204,18,RGB(60,30,36));bar=(int)(hp/maxhp*200);fill_rect(memdc,12,12,bar,14,RGB(70,220,105));
    fill_rect(memdc,10,34,204,8,RGB(34,48,70));bar=(int)((float)xp/xp_need*200);fill_rect(memdc,12,36,bar,4,RGB(70,190,255));

    for(i=0;i<MAX_E;i++)if(e[i].live){r=eradius(e[i].type);ellipse_c(memdc,(int)e[i].x+ox,(int)e[i].y+oy,r,e[i].type==BOSS?RGB(145,60,205):(e[i].type==ELITE?RGB(255,135,45):RGB(200,62,72)));if(e[i].type!=NORMAL){fill_rect(memdc,(int)e[i].x-r,(int)e[i].y-r-8,r*2,4,RGB(55,25,35));fill_rect(memdc,(int)e[i].x-r,(int)e[i].y-r-8,(int)(r*2*e[i].hp/e[i].maxhp),4,RGB(255,100,80));}}
    for(i=0;i<MAX_P;i++)if(p[i].live)ellipse_c(memdc,(int)p[i].x+ox,(int)p[i].y+oy,5,RGB(255,235,80));
    for(i=0;i<MAX_EP;i++)if(ep[i].live){ellipse_c(memdc,(int)ep[i].x+ox,(int)ep[i].y+oy,7,RGB(255,70,175));ring_c(memdc,(int)ep[i].x+ox,(int)ep[i].y+oy,10,RGB(255,110,210));}
    if(orbit_level)for(i=0;i<orbit_level+1&&i<6;i++){ang=time_alive*(2.2f+orbit_level*.12f)+i*2*PI/(orbit_level+1);bx=px+(50+orbit_level*3)*cosf(ang);by=py+(50+orbit_level*3)*sinf(ang);ellipse_c(memdc,(int)bx+ox,(int)by+oy,7,RGB(100,235,255));}
    if(nova_fx>0)ring_c(memdc,(int)px+ox,(int)py+oy,(int)((125+nova_level*16)*(1-nova_fx/.35f)),RGB(80,210,255));
    ellipse_c(memdc,(int)px+ox,(int)py+oy,14,invuln>0?RGB(150,215,255):RGB(65,155,255));ring_c(memdc,(int)px+ox,(int)py+oy,18,RGB(120,205,255));
    for(i=0;i<MAX_POP;i++)if(pops[i].on)ring_c(memdc,(int)pops[i].x+ox,(int)pops[i].y+oy,(int)(4+pops[i].t*32),pops[i].c);

    wsprintfA(buf,"HP %d/%d  LV %d  XP %d/%d",(int)hp,(int)maxhp,level,xp,xp_need);text(memdc,12,44,buf,RGB(232,238,248));
    wsprintfA(buf,"TIME %ds   KILLS %d   COINS %d",(int)time_alive,kills,coins);text(memdc,520,12,buf,RGB(232,238,248));
    wsprintfA(buf,"BOLT %d ORBIT %d NOVA %d  BOSSES %d RELICS %d",bolt_level,orbit_level,nova_level,bosses,relic_count());text(memdc,455,34,buf,RGB(120,205,255));
    text(memdc,12,H-22,"WASD/Arrows move   B shop   Auto-aim combat",RGB(145,158,178));
    if(boss_warning>0){wsprintfA(buf,"BOSS IN %d",(int)boss_warning+1);text(memdc,365,90,buf,RGB(255,105,170));}

    if(menu==MENU_LEVEL||menu==MENU_RELIC){fill_rect(memdc,72,188,656,230,RGB(17,21,31));text(memdc,316,210,menu==MENU_LEVEL?"LEVEL UP - CHOOSE A SKILL":"BOSS DEFEATED - CHOOSE A RELIC",menu==MENU_LEVEL?RGB(100,210,255):RGB(255,215,90));for(i=0;i<3;i++)draw_card(i,choice[i],menu==MENU_RELIC);}
    else if(menu==MENU_SHOP){fill_rect(memdc,164,152,472,294,RGB(17,21,31));text(memdc,350,174,"NIGHT MARKET",RGB(255,215,90));wsprintfA(buf,"COINS: %d",coins);text(memdc,350,200,buf,RGB(235,238,245));text(memdc,216,244,"[1] FIELD RATION    Heal 40                 12",RGB(185,240,195));text(memdc,216,284,"[2] SHARPENING      +4 weapon damage        20",RGB(255,195,120));text(memdc,216,324,"[3] HEART TONIC     +20 max HP and heal     24",RGB(135,215,255));text(memdc,216,382,"[4] or B - return to the battle",RGB(170,178,195));}
    if(game_over){fill_rect(memdc,205,210,390,174,RGB(18,22,31));text(memdc,350,236,"RUN ENDED",RGB(255,105,105));wsprintfA(buf,"SURVIVED %ds   KILLS %d",(int)time_alive,kills);text(memdc,305,272,buf,RGB(240,240,245));wsprintfA(buf,"LEVEL %d   BOSSES %d   SHOP BUYS %d",level,bosses,shop_buys);text(memdc,278,302,buf,RGB(180,205,235));text(memdc,314,344,"Press Space for a new run",RGB(255,235,190));}
    SelectObject(memdc,oldpen);BitBlt(dc,0,0,W,H,memdc,0,0,SRCCOPY);
}

static void make_backbuffer(HWND h){
    HDC dc=GetDC(h);BITMAPINFO bi;ZeroMemory(&bi,sizeof(bi));if(memdc){SelectObject(memdc,oldbmp);DeleteObject(dib);DeleteDC(memdc);}
    bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=W;bi.bmiHeader.biHeight=-H;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
    memdc=CreateCompatibleDC(dc);dib=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,(void**)&pix,0,0);oldbmp=(HBITMAP)SelectObject(memdc,dib);ReleaseDC(h,dc);
}

static LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){
    switch(m){
    case WM_CREATE:make_backbuffer(h);reset_game();QueryPerformanceFrequency(&freq);QueryPerformanceCounter(&last);SetTimer(h,1,16,0);return 0;
    case WM_DESTROY:if(memdc){SelectObject(memdc,oldbmp);DeleteObject(dib);DeleteDC(memdc);}PostQuitMessage(0);return 0;
    case WM_KILLFOCUS:ZeroMemory(keys,sizeof(keys));return 0;
    case WM_KEYDOWN:if(w<256){if(!keys[w])hits[w]=1;keys[w]=1;}if(w==VK_ESCAPE&&menu==MENU_NONE)DestroyWindow(h);return 0;
    case WM_KEYUP:if(w<256)keys[w]=0;return 0;
    case WM_TIMER:{LARGE_INTEGER now;float dt;QueryPerformanceCounter(&now);dt=(float)(now.QuadPart-last.QuadPart)/(float)freq.QuadPart;last=now;if(dt>.05f)dt=.05f;update_game(dt);ZeroMemory(hits,sizeof(hits));InvalidateRect(h,0,FALSE);return 0;}
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);render(dc);EndPaint(h,&ps);return 0;}
    }return DefWindowProcA(h,m,w,l);
}

int WINAPI WinMain(HINSTANCE hi,HINSTANCE hp,LPSTR cmd,int show){
    WNDCLASSA wc;RECT r={0,0,W,H};DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;(void)hp;(void)cmd;ZeroMemory(&wc,sizeof(wc));
    wc.lpfnWndProc=proc;wc.hInstance=hi;wc.lpszClassName="RalphthonGDI2";wc.hCursor=LoadCursor(0,IDC_ARROW);RegisterClassA(&wc);AdjustWindowRect(&r,style,FALSE);
    wnd=CreateWindowA("RalphthonGDI2","Ralphthon Survivor: Relic Run",style,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,0,0,hi,0);ShowWindow(wnd,show);UpdateWindow(wnd);
    for(;;){MSG msg;if(PeekMessageA(&msg,0,0,0,PM_REMOVE)){if(msg.message==WM_QUIT)return 0;TranslateMessage(&msg);DispatchMessageA(&msg);}else WaitMessage();}
}
