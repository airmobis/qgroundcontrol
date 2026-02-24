set(QGC_APP_NAME "Airmobis-QGroundControl")
set(QGC_APP_DESCRIPTION "Airmobis QGroundControl")
set(QGC_APP_COPYRIGHT "Copyright (C) 2025 Airmobis. All rights reserved.")
set(QGC_ORG_DOMAIN "com.airmobis")
set(QGC_ORG_NAME "Airmobis")
set(QGC_QT_ANDROID_MIN_SDK_VERSION "25")
set(QGC_QT_ANDROID_TARGET_SDK_VERSION "25")
set(QGC_ENABLE_HERELINK ON)

# Add Herelink AirUnit video configuration
add_compile_definitions(QGC_HERELINK_AIRUNIT_VIDEO)

# https://github.com/issues/created?issue=mavlink%7Cqgroundcontrol%7C14025
# TODO: Change Android package name via CMake once it's fixed upstream
#
# set(QGC_ANDROID_PACKAGE_NAME "com.airmobis.qgroundcontrol")
