# Developer notes -- Petr

_Many of these might be macOS-specific, as that’s where most of my development work takes place._

## Initial build
- GStreamer didn't provide its own `FindGStreamer.cmake` on macOS before 1.26, so both QGroundControl and Qt6 have their own
    - With 1.26, it's necessary to **prepend** its directory to `CMAKE_MODULE_PATH`, or the Qt version is tried first and fails
- However, CMake configuration fails with GStreamer 1.26 (see [this issue](https://github.com/mavlink/qgroundcontrol/issues/13049))
    - I had to downgrade to 1.24 via Homebrew commit `d59e37b5bebd3b7f8d6ddb6bc3483b8f9d3182d2`
    - However, this version doesn’t include a `FindGStreamer.cmake` file, so I copied it manually from 1.26 and it worked
