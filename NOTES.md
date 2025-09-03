# Developer notes (Petr)
_This file serves to document my experience working with and modifying QGroundControl 5 on my MacBook Air M2, as well as to record potentially helpful information I gathered along the way._

## Android build requirements
- Qt 6.6.3
- GStreamer 1.22.0-1.24.13
    - 1.26+ causes configure errors. Should be fixed soon enough by QGC maintainers.
- Android CLI Tools
    - Build Tools r35 (`build-tools;35.0.0`)
    - NDK 25.1 (`ndk;25.1.8937393`)
    - Platform Tools (`platform-tools`)
    - SDK Platform 34 (`platforms;android-34`)

## Initial build
- GStreamer didn't provide its own `FindGStreamer.cmake` on macOS before 1.26, so both QGroundControl and Qt6 have their own
    - With 1.26, it's necessary to **prepend** its directory to `CMAKE_MODULE_PATH`, or the Qt version gets tried first and fails.
- However, CMake configuration still fails due to unknown breaking changes in GStreamer 1.26 (see [this issue](https://github.com/mavlink/qgroundcontrol/issues/13049)).
    - I had to downgrade to 1.24 via Homebrew commit `d59e37b5bebd3b7f8d6ddb6bc3483b8f9d3182d2`.
    - However, this version doesn’t include a `FindGStreamer.cmake` file, so I copied it manually from 1.26, and it worked!

## Herelink plugin inclusion
- After adding the `custom` directory, two files had to be added to it for the custom build to successfully "register": `cmake/CustomOverrides.cmake` and `CMakeLists.txt`. This seems to indicate that QGroundControl 5 went the best-practice modular CMake route, which ought to simplify things.
- There are some caveats, though. The custom sources depend on QGC itself, so being "truly" modular probably isn't possible. Instead, the `CMakeLists.txt` file is responsible for not only creating its own libraries, but also creating variables that tell the top-level `CMakeLists.txt` which additional sources, include directories, and resources it should work with.
- Larger projects often come with their own rules, and it doesn't have to hurt as long as they document exactly how they stray from the beaten path (which QGC does). Either way, it's better to remember a few quirks than to go on a mass-refactoring rampage of the entire project in the name of "best practices." **We do not want to touch the core QGC project unless absolutely necessary**, see [this explanation](https://docs.qgroundcontrol.com/master/en/qgc-dev-guide/custom_build/fork_repo.html#modifying-mainline-qgc-source-code).

## clangd integration
- As mentioned above, the `custom` directory _seems_ like its own module, but it's really dependent on QGC's APIs, which confuses clangd. To ease the pain, I decided to replace the `CMakeLists.txt` with a `config.cmake` file, which essentially runs the same logic, but ensures language server integration with the entire project.
    - A modification in the top-level `CMakeLists.txt` was required&mdash;I replaced `add_subdirectory(custom)` with `include(custom/config.cmake)` on line 261.

## Porting the Herelink plugin
- The `cameraId` parameter seems to have been changed from an integer type to a `QString`.
- Plugins and other manager classes are now accessible via a static `instance()` method.
- There were some undefined symbols during linking&mdash;turns out Qt's MOC needs header files to be added as target sources as well. Compilation was sucessful after that[^1].
- Afterwards, the compiled `QGroundControl-herelink.app` would crash right after launching; I tracked it down to `SettingsFact.cc:37`, where it became clear that the crash was due to `CustomPlugin` not overriding `QGCCorePlugin::instance()`[^2].

> [!CAUTION]
> The QGC-Herelink project made changes in QGC's source code to create and connect their own `VideoStreamControl`, specifically in `src/VideoManager/VideoManager{.h, .cc}`.
> I am unsure as to whether this is necessary for the port... stay tuned&trade;.

## Configuring for Android
- QGroundControl 5 migrated to pure CMake, so we'll try and do the same, instead of going down a rabbit hole with Qt Creator.
    - After installing the Android toolchain along with NDK r26b and build-tools 26.3, everything seemed OK, but the project has one last error up its sleeve&mdash;Qt6LinguistTools could apparently not be located! After some hair-tearing, I finally found the fix: Including LinguistTools by themselves in QGC's `CMakeLists.txt` fixed the problem.
    - The APK refused to build because my `sdkmanager` packages were outdated (min. `build-tools;35.0.0` and `platforms;android-34`). Easy enough to upgrade.

## Changes from upstream (excl. custom directory)
_These can also be queried via `git diff upstream/Stable_V5.0 origin/Stable_V5.0`, but it's nice to have a concise list._
- Added Android GStreamer directory to `.gitignore`
- Added `NOTES.md` (this file)
- Modified `README.md`
- Modified `find_package()` logic for GStreamer in `src/VideoManager/VideoReceiver/GStreamer/gstqml6gl/CMakeLists.txt`
- Modified `CMakeLists.txt` to include Linguist instead of LinguistTools[^3]
- Removed `cmake/modules/FindGStreamer.cmake`
- Removed `QGC_CPM_SOURCE_CACHE`[^4]

[^1]: Fixed in commit 139d4109d740aa7eb96b23e68ec3738f59a7d97f.
[^2]: Fixed in commit aba10c93ee2056866fc87b7f1afe09a38d401616.
[^3]: Modified in commit 5e3feac9ab615fdcacdd2f009871f655cc3f3e76
[^4]: Removed in commit e41985c7decf225f9cff198ccc9efd5fd2f2ff4c.
