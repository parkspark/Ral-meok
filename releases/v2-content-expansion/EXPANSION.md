# Content Expansion Plan

## Product direction

Turn the verified MVP into a compact run-based action roguelite without
changing the native Windows, single-EXE, no-assets constraints.

## Connected progression loop

1. Normal enemies provide steady XP and coins.
2. XP levels open a paused three-choice skill draft.
3. Elites appear on a timer and provide accelerated rewards.
4. A boss arrives every 60 seconds and fires aimed projectiles.
5. Boss kills open a three-choice relic draft.
6. Coins are spent in the in-run shop, opened with `B`.
7. Death clears the run build; Space starts a fresh randomized run.

## Content

- Skills: Arc Bolt, Orbit Blades, Nova Pulse, Power, Fleet, Fortify.
- Relics: Vital Core, Chrono Lens, Wind Boots, Bloodstone, Thorn Crown,
  Lucky Coin.
- Enemies: swarmers, tougher elites, large ranged bosses.
- Shop: healing, permanent run damage, permanent run maximum HP.
- Feedback: elite/boss silhouettes and HP bars, boss warning, XP bar,
  currency/build HUD, choice cards, nova/orbit effects.

## Guardrails

- Fixed-size pools only; no per-frame allocation.
- GDI primitives and Windows stock fonts only.
- No external files in the runtime payload.
- Preserve the original three difficulty bands and all MVP behavior.
- Rebuild and measure after implementation; release only if the same final EXE
  passes launch, input, two-minute stability, dependency, and byte gates.
