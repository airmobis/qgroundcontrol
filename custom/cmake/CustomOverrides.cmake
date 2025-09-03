set(QGC_APP_NAME "QGroundControl-Herelink" CACHE STRING "App Name" FORCE)
set(QGC_APP_COPYRIGHT "Copyright (c) 2025 Airmobis. All rights reserved." CACHE STRING "Copyright" FORCE)
set(QGC_APP_DESCRIPTION "Open Source Ground Control App (Modified for Herelink)" CACHE STRING "Description" FORCE)
set(QGC_ORG_NAME "Airmobis" CACHE STRING "Org Name" FORCE)
set(QGC_ORG_DOMAIN "airmobis.com" CACHE STRING "Domain" FORCE)
set(QGC_PACKAGE_NAME "com.airmobis.qgroundcontrol" CACHE STRING "Package Name" FORCE)
set(QGC_ANDROID_PACKAGE_NAME "com.airmobis.qgroundcontrol" CACHE STRING "Android Package Name" FORCE)

set(QGC_ENABLE_BLUETOOTH OFF CACHE BOOL "Enable Bluetooth Links"  FORCE)
set(QGC_AIRLINK_DISABLED OFF CACHE BOOL "Disable AIRLink" FORCE)

# Compilation fails with this set. It could probably be fixed, but it'd
# take a truckload of #if(n)def-s.
# set(QGC_NO_SERIAL_LINK ON CACHE BOOL "Disable Serial Links" FORCE)
