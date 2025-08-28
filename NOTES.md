# Developer notes -- Petr

_This file serves to document my experience working with and modifying QGroundControl 5 on my MacBook Air M2, as well as to record potentially helpful information I gathered along the way._

## Initial build
- GStreamer didn't provide its own `FindGStreamer.cmake` on macOS before 1.26, so both QGroundControl and Qt6 have their own
    - With 1.26, it's necessary to **prepend** its directory to `CMAKE_MODULE_PATH`, or the Qt version is tried first and fails
- However, CMake configuration fails still with GStreamer 1.26 (see [this issue](https://github.com/mavlink/qgroundcontrol/issues/13049))
    - I had to downgrade to 1.24 via Homebrew commit `d59e37b5bebd3b7f8d6ddb6bc3483b8f9d3182d2`
    - However, this version doesn’t include a `FindGStreamer.cmake` file, so I copied it manually from 1.26 and it worked

## Herelink plugin inclusion
- After adding the `custom` directory, two files had to be added for the custom build to successfully "register": `cmake/CustomOverrides.cmake` and `CMakeLists.txt`. This seems to indicate that QGroundControl 5 went the best-practice modular CMake route, which ought to simplify things.

## TODO
- Test the (non-custom) build on a work PC
