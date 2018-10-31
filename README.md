[![Build status](https://ci.appveyor.com/api/projects/status/c61oe1cjcs593ufm/branch/master?svg=true)](https://ci.appveyor.com/project/tmp64/bugfixedhl/branch/master)

# Bugfixed and improved HL release
Bugfixed and improved HLSDK release.
This was started in a purpose to replace bugged hl.dll on a server to prevent frequent server crashes.
Now it's a lot of fixes and improvements to server and client sides.
Based on HLSDK version 2.3-p3 patched by metamod team.

## Downloads
Releases are available on [GitHub Releases page](https://github.com/tmp64/BugfixedHL/releases).

Development builds can be found on project's [AppVeyor page](https://ci.appveyor.com/project/tmp64/bugfixedhl/branch/master).
They may be unstable or may not work at all.

## VGUI2
VGUI2 is a UI library by Valve for Steam and Source Engine.
It is used in Half-Life for its game menu (console, settings, create server dialogs and etc)
and in CS1.6 for in-game menus and scoreboard.

Half-Life uses VGUI1 for in-game scoreboard, MOTD dialog and spectator HUD.
VGUI1 has some limitations over VGUI2:
- Supports only English (Latin-1 encoding while VGUI2 uses UTF-16)
- Has no public documentation
- Completely closed-source

### VGUI2 elements
- [ ] Scoreboard - Work In Progress
- [ ] Chatbox
- [ ] Unicode MOTD
- [ ] HTML MOTD
- [ ] Custom crosshairs

## Thanks
- Lev for creating [the original BFAIHLSDK](https://github.com/LevShisterov/BugfixedHL)
- Valve for HLSDK release.
- SamVanheer for [Half-Life Enhanced](https://github.com/SamVanheer/HLEnhanced)
- Willday for his HLSDK patch.
- BubbleMod and Bigguy from hlpp.thewavelength.net for parts of spectator code.
- Uncle Mike from hlfx.ru for his Xash3D engine which was very helpful in hard moments.
- KORD_12.7 for constant helping and nice suggestions.
- AGHL.RU community for bug reporting and suggestions.
- JetBrains company for free access to great developer tools.

## Installation
> **Warning!** VGUI2 version only supports new builds ( from Jul 7 2017 (7561) )!
> Use `version` command to get your game version.
- Download latest release archive from the [Releases](https://github.com/tmp64/BugfixedHL/releases) page
- Copy all files from release archive "valve" folder to server/client "valve" folder replacing all.
- Steam: copy to &lt;Steam Installation or Library Directory&gt;\steamapps\common\Half-Life\valve
- No-Steam: copy to &lt;game folder&gt;\valve

## Support forum URL
http://aghl.ru/forum/viewtopic.php?f=36&t=686
