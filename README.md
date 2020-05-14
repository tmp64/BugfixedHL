[![Build status](https://ci.appveyor.com/api/projects/status/c61oe1cjcs593ufm/branch/multimode?svg=true)](https://ci.appveyor.com/project/tmp64/bugfixedhl/branch/multimode)

# Half-Life Multimode
Half-Life Multimode is a gamemode where the game is split into short rounds which feature different modes that affect gameplay.

Currently implemented modes:
- OneShot-Gun - shotgun is given on spawn and one-shots everyone;
- Extreme Recoil - MP5, shotgun and revolver have huge knockback, players are able to fly with these weapons;
- Heavy Weapons - revolver is given on spawn and after every shot it is dropped;
- Biohazard - only snarks and hornet are available;
- Slow Rockets - everyone has RPG but rockets fly very slow and do more damage.
- Ludicrous Speed - [Light speed is too slow.](https://www.youtube.com/watch?v=ygE01sOhzz0) You're gonna have to run at constant ~~ridiculous~~ ludicrous speed.
- Boss Fight - one player becomes The Boss with all weapons and a lot of HP, other players must kill them.


## Downloads
Latest builds can be found on the [AppVeyor page](https://ci.appveyor.com/project/tmp64/bugfixedhl/branch/multimode).


## Installation
1. Download the archive from the link above for your platform.
2. Extract contents of *gamedir/* to *valve*
3. Create (if it doesn't exist) *valve/game.cfg* with following contents:
    ```c
    mp_multimode 1
    // Other mp_mm_... cvars go here.
    ```


## Compatibility
The mod is fully compatible with Metamod and partially with AMXX.
- Most AMXX plugins should work fine.
- GameRules natives don't work on Windows (AMXX bug (kind of)).
- GameRules natives may corrupt the memory when running Team Deathmatch (*mp_teamplay 1*) (use BugfixedHL for that instead).
- Some entity offsets may change in the future.


## Configuration
Almost all configuration is located in *hl_multimode.json* along with comments. Some options are configured with convars.

| Convar | Value | Description |
| ------ | ----- | ----------- |
| mp_multimode | 0/**1** | Enable Multimode gamemode. |
| mp_fix_mp5_ammo | 0/**1** | MP5 wil have full magazine (50) on pick up (instead of 25) |
| mp_mm_config_file | string **hl_multimode.json** | Name of the config file for Multimode. |
| mp_mm_skip_warmup | 0/1 | If 1, the warm-up will end immediately and cvar will be reset to 0. (for debugging) |
| mp_mm_skip_mode | 0/1 | If 1, the game will skip current mode and go to the next, cvar will be reset to 0. (for debugging) |

## Thanks
- Lev for creating [the original BFAIHLSDK](https://github.com/LevShisterov/BugfixedHL).
- Valve for HLSDK release.
- SamVanheer for [Half-Life Enhanced](https://github.com/SamVanheer/HLEnhanced) and reverse engineering VGUI2 for GoldSrc.
- Willday for his HLSDK patch.
- BubbleMod and Bigguy from hlpp.thewavelength.net for parts of spectator code.
- Uncle Mike from hlfx.ru for his Xash3D engine which was very helpful in hard moments.
- KORD_12.7 for constant helping and nice suggestions.
- AGHL.RU community for bug reporting and suggestions.
- JetBrains company for free access to great developer tools.


## Reporting problems
https://github.com/tmp64/BugfixedHL/issues
