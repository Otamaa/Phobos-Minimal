[![license](https://img.shields.io/github/license/Phobos-developers/Phobos.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)

# Phobos - Minimal
Is an fork of a community engine extension [project](https://github.com/Phobos-developers/Phobos) providing a set of new features and fixes for Yuri's Revenge based on [modified YRpp](https://github.com/Metadorius/YRpp) and [Syringe](https://github.com/Ares-Developers/Syringe) to allow injecting code. It's meant to take over some part of [Ares](https://github.com/Ares-Developers/Ares) mainly version 3.0p1.
This Phobos version also include edited version of [yrpp-spawner](https://github.com/CnCNet/yrpp-spawner/) 

# Compatibility 
- Compatibility is not guaranteed ouside common dll(s) that not interecting direcly with the game executable , it is take time and effort to make this dll compatible with other dll(s).
- This version of Phobos is not require Ares to function.

Currently Missing Feature(s) from Ares 3.0p1: 
 - Fixed game image screenshoot
 - Ingame menu UI (this not backported due to mod nowdays is using client anyways)
 - Some minor bugfix or optimization that probably still missing , visit the googledocument spreadsheet for more
 - Random Map Generator

# Stability 
As the main developer is still in learning phase and constanly changing the code. The stability of this version of Phobos is not guaranteed. Always make sure backup your project before fully integrating this dll version .

Building manually
-----------------

0. Install **Visual Studio** (2022 is recommended, 2019 is minimum).
1. Clone this repo recursively via your favorite git client (that will also clone YRpp).
2. To build the extension:
   - Open `Phobos.sln`;
   - **Visual Studio** will prompt you to install needed components if not installed yet , just follow it until is done;
   - in **Visual Studio**: make sure it is on `Build` and `x86` arcitecture and press `F7` to build the project;
3. Upon build completion the resulting `Phobos.dll` and `Phobos.pdb` would be placed in the `Build/` folder.
4. Copy `Syringe.exe` included or use any `Syringe.exe` that compatible with the dll onto Game folder along with the builded dll

Credits
-------

### Developers
- **Otamaa (Fahroni, BoredEXE)** - Main developer of this fork version ([PayPal](https://paypal.me/GeneralOtama))

## Special Thanks 
- **deathreaperz** - feature ideas , extensive and thorough testing
- **pharell23** -  feature ideas , help with extensive and thorough testing
- **metalleger_2412** - help with , extensive and thorough testing
- **mirceaofrivia** -  feature ideas , extensive and thorough testing
- **ayylmao** - help with small docs ,  feature ideas , extensive and thorough testing
- **solacex** - extensive and thorough testing
- **Rise of the East community** - extensive playtesting of in-dev features

Legal and License
-----

[![LGPL v3](https://www.gnu.org/graphics/lgplv3-147x51.png)](https://opensource.org/licenses/LGPL-3.0)

The Phobos project is an unofficial open-source community collaboration project to extend the Red Alert 2 Yuri's Revenge engine for modding and compatibility purposes.

This project has no direct affiliation with Electronic Arts Inc. Command & Conquer, Command & Conquer Red Alert 2, Command & Conquer Yuri's Revenge are registered trademarks of Electronic Arts Inc. All Rights Reserved.
