set(QGC_APP_NAME "QGroundControl-Herelink")
set(QGC_APP_DESCRIPTION "QGroundControl Herelink")
set(QGC_APP_COPYRIGHT "Copyright (C) 2025 Cubepilot. All rights reserved.")
set(QGC_ORG_DOMAIN "org.cubepilot")
set(QGC_ORG_NAME "Cubepilot")
set(QGC_QT_ANDROID_MIN_SDK_VERSION "25")
set(QGC_QT_ANDROID_TARGET_SDK_VERSION "25")
set(QGC_ENABLE_HERELINK ON)

# Add Herelink AirUnit video configuration
add_compile_definitions(QGC_HERELINK_AIRUNIT_VIDEO)
