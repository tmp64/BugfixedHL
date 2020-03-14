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
| Convar | Value | Description |
| ------ | ----- | ----------- |
| mp_fix_mp5_ammo | 0/**1** | MP5 wil have full magazine (50) on pick up (instead of 25) |
| mp_mm_min_players | integer **(1)** | Minimum count of player for game to switch to warm-up. |
| mp_mm_warmup_time | seconds **(45)** | Time it takes for warm-up to end. |
| mp_mm_freeze_time | seconds **(5)** | Time after players have been spawned but before the mode begins. |
| mp_mm_game_time | seconds **(60)** | Time of each round. |
| mp_mm_skip_warmup | 0/1 | If 1, the warm-up will end immediately and cvar will be reset to 0. (for debugging) |
| mp_mm_skip_mode | 0/1 | If 1, the game will skip current mode and go to the next, cvar will be reset to 0. (for debugging) |
| mp_mm_on_end | **0**/1/2 | 0: the game will continue from the first mode;<br>1: the game will go in intermission;<br>2: the game will go in "endgame" state to choose a new map using mapchooser_multimode AMXX plugin (based on mapchooser.sma, not yet released). |
| mp_mm_wpndrop_respawn | seconds **(7)** | Heavy Weapons: How long it takes for revolver to be regiven. |
| mp_mm_wpndrop_infammo | 0/**1** | Heavy Weapons: Should revolver have infinite ammo |
| mp_mm_wpndrop_rndangle | 0-180 **(60)** | Heavy Weapons: Random angle variation |
| mp_mm_biohaz_snark_respawn | seconds **(10)** | Biohazard: How long it takes for snarks to be regiven. |
| mp_mm_rocket_respawn | seconds **(10)** | Slow Rockets: How long it takes for rockets to be regiven. |
| mp_mm_rocket_speed | u/s **(120)** | Slow Rockets: Speed of the rockets. |
| mp_mm_speed_maxspeed | u/s **(600)** | Ludicrous Speed: Max horizontal speed of the players. |
| mp_mm_speed_accel | u/s/t **(50)** | Ludicrous Speed: At which rate the speed increases (units per second per tick) |


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
