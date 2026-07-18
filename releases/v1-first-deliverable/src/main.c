#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#define W 800
#define H 600
#define MAX_E 160
#define MAX_P 96

typedef struct { float x,y,hp,spd; int live; } Enemy;
typedef struct { float x,y,vx,vy,life; int live,dmg; } Shot;
typedef struct { int on; float x,y,t; COLORREF c; } Pop;

static HWND wnd;
static HDC memdc;
static HBITMAP dib, oldbmp;
static uint32_t *pix;
static Enemy e[MAX_E];
static Shot p[MAX_P];
static Pop pops[24];
static float px,py,hp,invuln,fire_cd,spawn_cd,time_alive,shake;
static int kills,game_over,upgrade,keys[256];
static uint32_t rng=0x31415926u;
static LARGE_INTEGER freq,last;

static uint32_t rndu(void){ rng=rng*1664525u+1013904223u; return rng; }
static float rndf(float a,float b){ return a+(float)(rndu()&65535)/65535.0f*(b-a); }
static float invsqrt(float x){ float y=x>1.0f?x:1.0f; int i; if(x<=0.0001f) return 0.0f; for(i=0;i<12;i++) y=.5f*(y+x/y); return 1.0f/y; }
static void addpop(float x,float y,COLORREF c){ int i; for(i=0;i<24;i++) if(!pops[i].on){ pops[i].on=1;pops[i].x=x;pops[i].y=y;pops[i].t=.25f;pops[i].c=c;return; } }

static void reset_game(void){
    int i; for(i=0;i<MAX_E;i++) e[i].live=0; for(i=0;i<MAX_P;i++) p[i].live=0; for(i=0;i<24;i++) pops[i].on=0;
    px=W*.5f; py=H*.5f; hp=100; invuln=0; fire_cd=0; spawn_cd=.2f; time_alive=0; kills=0; upgrade=0; game_over=0; shake=0;
}

static void spawn_enemy(void){
    int i,side; float x=0,y=0;
    for(i=0;i<MAX_E;i++) if(!e[i].live){
        side=(int)(rndu()%4);
        if(side==0){x=rndf(0,W);y=-20;} else if(side==1){x=W+20;y=rndf(0,H);}
        else if(side==2){x=rndf(0,W);y=H+20;} else {x=-20;y=rndf(0,H);}
        e[i].live=1; e[i].x=x; e[i].y=y; e[i].hp=18+upgrade*4;
        e[i].spd=42+(time_alive>30?5:0)+(time_alive>60?12:0)+rndf(0,16);
        return;
    }
}

static void fire_shot(void){
    int i,best=-1; float bd=99999999.0f,dx,dy,d2,iv;
    for(i=0;i<MAX_E;i++) if(e[i].live){
        dx=e[i].x-px; dy=e[i].y-py; d2=dx*dx+dy*dy;
        if(d2<bd){ bd=d2; best=i; }
    }
    if(best<0) return;
    for(i=0;i<MAX_P;i++) if(!p[i].live){
        dx=e[best].x-px; dy=e[best].y-py; iv=invsqrt(dx*dx+dy*dy);
        p[i].live=1; p[i].x=px; p[i].y=py; p[i].vx=dx*iv*440.0f; p[i].vy=dy*iv*440.0f;
        p[i].life=1.6f; p[i].dmg=12+upgrade*5; addpop(px,py,RGB(255,245,90)); return;
    }
}

static void update_game(float dt){
    int i,j,stage,threshold; float mx=0,my=0,iv,dx,dy,d2,interval,pspd=190.0f;
    if(game_over){ if(keys[VK_SPACE]) reset_game(); return; }
    time_alive+=dt; if(invuln>0) invuln-=dt; if(fire_cd>0) fire_cd-=dt; if(shake>0) shake-=dt;

    if(keys['A']||keys[VK_LEFT]) mx-=1; if(keys['D']||keys[VK_RIGHT]) mx+=1;
    if(keys['W']||keys[VK_UP]) my-=1; if(keys['S']||keys[VK_DOWN]) my+=1;
    if(mx&&my){ mx*=.7071067f; my*=.7071067f; }
    px+=mx*pspd*dt; py+=my*pspd*dt;
    if(px<14)px=14; if(px>W-14)px=W-14; if(py<14)py=14; if(py>H-14)py=H-14;

    stage=time_alive>=60?2:(time_alive>=30?1:0);
    interval=stage==0?1.5f:(stage==1?1.0f:.6f);
    spawn_cd-=dt; while(spawn_cd<=0){ spawn_enemy(); spawn_cd+=interval; if(stage==2) spawn_enemy(); }

    for(i=0;i<MAX_E;i++) if(e[i].live){
        dx=px-e[i].x; dy=py-e[i].y; iv=invsqrt(dx*dx+dy*dy);
        e[i].x+=dx*iv*e[i].spd*(stage==1?1.1f:(stage==2?1.2f:1.0f))*dt;
        e[i].y+=dy*iv*e[i].spd*(stage==1?1.1f:(stage==2?1.2f:1.0f))*dt;
        d2=(e[i].x-px)*(e[i].x-px)+(e[i].y-py)*(e[i].y-py);
        if(d2<25*25 && invuln<=0){ hp-=12; invuln=.5f; shake=.18f; addpop(px,py,RGB(255,80,70)); if(hp<=0){ hp=0; game_over=1; } }
    }

    if(fire_cd<=0){ fire_shot(); fire_cd=(.55f-upgrade*.055f); if(fire_cd<.18f) fire_cd=.18f; }
    for(i=0;i<MAX_P;i++) if(p[i].live){
        p[i].x+=p[i].vx*dt; p[i].y+=p[i].vy*dt; p[i].life-=dt;
        if(p[i].life<=0 || p[i].x<-30 || p[i].x>W+30 || p[i].y<-30 || p[i].y>H+30) p[i].live=0;
        if(!p[i].live) continue;
        for(j=0;j<MAX_E;j++) if(e[j].live){
            dx=e[j].x-p[i].x; dy=e[j].y-p[i].y; if(dx*dx+dy*dy<18*18){
                e[j].hp-=p[i].dmg; p[i].live=0; addpop(e[j].x,e[j].y,RGB(255,150,70));
                if(e[j].hp<=0){ e[j].live=0; kills++; addpop(e[j].x,e[j].y,RGB(120,255,150)); }
                break;
            }
        }
    }
    threshold=10+upgrade*12; if(kills>=threshold && upgrade<8){ upgrade++; addpop(px,py,RGB(80,220,255)); }
    for(i=0;i<24;i++) if(pops[i].on){ pops[i].t-=dt; if(pops[i].t<=0) pops[i].on=0; }
}

static void fill_rect(HDC dc,int x,int y,int w,int h,COLORREF c){ RECT r={x,y,x+w,y+h}; SetDCBrushColor(dc,c); FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH)); }
static void ellipse_c(HDC dc,int x,int y,int r,COLORREF c){ SetDCBrushColor(dc,c); SelectObject(dc,GetStockObject(DC_BRUSH)); Ellipse(dc,x-r,y-r,x+r,y+r); }
static void text(HDC dc,int x,int y,const char*s,COLORREF c){ SetTextColor(dc,c); SetBkMode(dc,TRANSPARENT); TextOutA(dc,x,y,s,lstrlenA(s)); }

static void render(HDC dc){
    int i,ox=0,oy=0,bar; char buf[96]; HPEN oldpen=(HPEN)SelectObject(memdc,GetStockObject(NULL_PEN));
    if(shake>0){ ox=(int)(rndf(-3,3)); oy=(int)(rndf(-3,3)); }
    fill_rect(memdc,0,0,W,H,RGB(10,12,18));
    for(i=0;i<W;i+=40) fill_rect(memdc,i+ox,0,1,H,RGB(22,26,35));
    for(i=0;i<H;i+=40) fill_rect(memdc,0,i+oy,W,1,RGB(22,26,35));
    fill_rect(memdc,10,10,204,18,RGB(60,30,36)); bar=(int)(hp*2); if(bar<0)bar=0; fill_rect(memdc,12,12,bar,14,RGB(70,220,105));

    for(i=0;i<MAX_E;i++) if(e[i].live) ellipse_c(memdc,(int)e[i].x+ox,(int)e[i].y+oy,13,RGB(200,62,72));
    for(i=0;i<MAX_P;i++) if(p[i].live) ellipse_c(memdc,(int)p[i].x+ox,(int)p[i].y+oy,5,RGB(255,235,80));
    ellipse_c(memdc,(int)px+ox,(int)py+oy,14,invuln>0?RGB(115,190,255):RGB(80,165,255));
    for(i=0;i<24;i++) if(pops[i].on) ellipse_c(memdc,(int)pops[i].x+ox,(int)pops[i].y+oy,(int)(4+pops[i].t*28),pops[i].c);

    wsprintfA(buf,"HP %d  TIME %ds  KILLS %d  POWER %d",(int)hp,(int)time_alive,kills,upgrade+1); text(memdc,12,34,buf,RGB(235,238,245));
    text(memdc,12,H-24,"WASD/Arrows move - auto fire nearest enemy",RGB(150,160,176));
    if(game_over){
        fill_rect(memdc,220,230,360,104,RGB(20,22,30));
        wsprintfA(buf,"GAME OVER - SURVIVED %d SECONDS",(int)time_alive); text(memdc,286,258,buf,RGB(255,235,190));
        text(memdc,320,290,"Press Space to restart",RGB(235,238,245));
    }
    SelectObject(memdc,oldpen);
    BitBlt(dc,0,0,W,H,memdc,0,0,SRCCOPY);
}

static void make_backbuffer(HWND h){
    HDC dc=GetDC(h); BITMAPINFO bi; ZeroMemory(&bi,sizeof(bi));
    if(memdc){ SelectObject(memdc,oldbmp); DeleteObject(dib); DeleteDC(memdc); }
    bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER); bi.bmiHeader.biWidth=W; bi.bmiHeader.biHeight=-H;
    bi.bmiHeader.biPlanes=1; bi.bmiHeader.biBitCount=32; bi.bmiHeader.biCompression=BI_RGB;
    memdc=CreateCompatibleDC(dc); dib=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,(void**)&pix,0,0); oldbmp=(HBITMAP)SelectObject(memdc,dib); ReleaseDC(h,dc);
}

static LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){
    switch(m){
    case WM_CREATE: make_backbuffer(h); reset_game(); QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&last); SetTimer(h,1,16,0); return 0;
    case WM_DESTROY: if(memdc){ SelectObject(memdc,oldbmp); DeleteObject(dib); DeleteDC(memdc); } PostQuitMessage(0); return 0;
    case WM_KEYDOWN: if(w<256) keys[w]=1; if(w==VK_ESCAPE) DestroyWindow(h); return 0;
    case WM_KEYUP: if(w<256) keys[w]=0; return 0;
    case WM_TIMER: {
        LARGE_INTEGER now; float dt; QueryPerformanceCounter(&now); dt=(float)(now.QuadPart-last.QuadPart)/(float)freq.QuadPart; last=now;
        if(dt>.05f) dt=.05f; update_game(dt); InvalidateRect(h,0,FALSE); return 0; }
    case WM_PAINT: { PAINTSTRUCT ps; HDC dc=BeginPaint(h,&ps); render(dc); EndPaint(h,&ps); return 0; }
    }
    return DefWindowProcA(h,m,w,l);
}

int WINAPI WinMain(HINSTANCE hi,HINSTANCE hp,LPSTR cmd,int show){
    WNDCLASSA wc; RECT r={0,0,W,H}; DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
    (void)hp; (void)cmd; ZeroMemory(&wc,sizeof(wc));
    wc.lpfnWndProc=proc; wc.hInstance=hi; wc.lpszClassName="RalphthonGDI"; wc.hCursor=LoadCursor(0,IDC_ARROW);
    RegisterClassA(&wc); AdjustWindowRect(&r,style,FALSE);
    wnd=CreateWindowA("RalphthonGDI","Ralphthon Survivor",style,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,0,0,hi,0);
    ShowWindow(wnd,show); UpdateWindow(wnd);
    for(;;){ MSG msg; if(PeekMessageA(&msg,0,0,0,PM_REMOVE)){ if(msg.message==WM_QUIT) return 0; TranslateMessage(&msg); DispatchMessageA(&msg); } else WaitMessage(); }
}
