[![license](https://img.shields.io/github/license/Phobos-developers/Phobos.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)

# Phobos - Minimal
Is an fork of a community engine extension [project](https://github.com/Phobos-developers/Phobos) providing a set of new features and fixes for Yuri's Revenge based on [modified YRpp](https://github.com/Metadorius/YRpp) and [Syringe](https://github.com/Ares-Developers/Syringe) to allow injecting code. It's meant to take over some part of [Ares](https://github.com/Ares-Developers/Ares) mainly version 3.0p1.

# Dll Compatibility 
Compatibility is not guaranteed ouside Ares 3.0p1 because it is take time and effort to fill the gap for other version of Ares and or other dll outside of Ares.
this version of Phobos is require Ares 3.0p1 to function.

# Stability 
As the main developer is still in learning phase and constanly changing the code. The stability of this version of Phobos is not guaranteed. Always make sure backup your project before fully integrating this dll version .

Building manually
-----------------

0. Install **Visual Studio** (2019 is recommended, 2017 is minimum) with the dependencies listed in `.vsconfig` (it will prompt you to install missing dependences when you open the project, or you can run VS installer and import the config). If you prefer to use **Visual Studio Code** you may install **VS Build Tools** with the dependencies from `.vsconfig` instead. You can also don't use any code editor or IDE and build via **command line scripts** included with the project.
1. Clone this repo recursively via your favorite git client (that will also clone YRpp).
2. To build the extension:
   - in Visual Studio: open the solution file in VS and build it (`Debug` build config is recommended);
   - in VSCode: open the project folder and hit `Run Build Task...` (`Ctrl + Shift + B`);
   - barebones: run `scripts/build_debug.bat`.
3. Upon build completion the resulting `Phobos.dll` and `Phobos.pdb` would be placed in the subfolder identical to the name of the buildconfig executed.

Credits
-------

### Developers
- **Otamaa (Fahroni, BoredEXE)** - Main developer of this fork version ([PayPal](https://paypal.me/GeneralOtama))
- **Belonit (Gluk-v48)** - project author (retired)
- **Kerbiter (Metadorius)** - project co-author, project maintainer (retired) ([Patreon](http://patreon.com/kerbiter))
- **Uranusian (Thrifinesma)** - developer, CN community ambassador ([Patreon](https://www.patreon.com/uranusian), [AliPay](http://tiebapic.baidu.com/forum/w%3D580/sign=4b04b953307f9e2f70351d002f31e962/b3f89909b3de9c823bd7f23a7b81800a18d84371.jpg))
- **secsome (SEC-SOME)** - developer
- **FS-21** - developer
- **Starkku** - developer
- **Morton (MortonPL)** - developer
- **and more !

Thanks to everyone who uses Phobos, tests changes and reports bugs! You can show your appreciation and help project by displaying the logo (monochrome version can be found [here](logo-mono.png)) in your client/launcher (make it open Phobos GitHub page for extra fanciness), linking to Phobos repository, contributing or donating to us via the links above.

Legal and License
-----

[![LGPL v3](https://www.gnu.org/graphics/lgplv3-147x51.png)](https://opensource.org/licenses/LGPL-3.0)

The Phobos project is an unofficial open-source community collaboration project to extend the Red Alert 2 Yuri's Revenge engine for modding and compatibility purposes.

This project has no direct affiliation with Electronic Arts Inc. Command & Conquer, Command & Conquer Red Alert 2, Command & Conquer Yuri's Revenge are registered trademarks of Electronic Arts Inc. All Rights Reserved.
