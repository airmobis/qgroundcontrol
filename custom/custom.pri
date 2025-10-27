message("Adding Custom GeoWork Plugin")

#-- Version control
#   Major and minor versions are defined here (manually)

CUSTOM_QGC_VER_MAJOR = 4
CUSTOM_QGC_VER_MINOR = 4
CUSTOM_QGC_VER_PATCH = 5
CUSTOM_QGC_VER_FIRST_BUILD = 0

linux {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-strict-aliasing
}

# Build number is automatic
# Uses the current branch. This way it works on any branch including build-server's PR branches
CUSTOM_QGC_VER_BUILD = $$system(git --git-dir ../.git rev-list $$GIT_BRANCH --first-parent --count)

win32 {
    CUSTOM_QGC_VER_BUILD = $$system("set /a $$CUSTOM_QGC_VER_BUILD - $$CUSTOM_QGC_VER_FIRST_BUILD")
} else {
    CUSTOM_QGC_VER_BUILD = $$system("echo $(($$CUSTOM_QGC_VER_BUILD - $$CUSTOM_QGC_VER_FIRST_BUILD))")
}

CUSTOM_QGC_VERSION = $${CUSTOM_QGC_VER_MAJOR}.$${CUSTOM_QGC_VER_MINOR}.$${CUSTOM_QGC_VER_PATCH}.$${CUSTOM_QGC_VER_BUILD}

DEFINES -= APP_VERSION_STR=\"\\\"$$APP_VERSION_STR\\\"\"
DEFINES += APP_VERSION_STR=\"\\\"$$CUSTOM_QGC_VERSION\\\"\"

message(Custom QGC Version: $${CUSTOM_QGC_VERSION})

# Branding

DEFINES += CUSTOMHEADER=\"\\\"CustomPlugin.h\\\"\"
DEFINES += CUSTOMCLASS=CustomPlugin

TARGET   = Airmobis-QGroundControl
DEFINES += QGC_APPLICATION_NAME='"\\\"Airmobis QGroundControl\\\""'

DEFINES += QGC_ORG_NAME=\"\\\"airmobis.com\\\"\"
DEFINES += QGC_ORG_DOMAIN=\"\\\"com.airmobis\\\"\"

QGC_APP_NAME        = "Airmobis QGroundControl"
QGC_BINARY_NAME     = "Airmobis-QGroundControl"
QGC_ORG_NAME        = "Airmobis"
QGC_ORG_DOMAIN      = "com.airmobis"
QGC_ANDROID_PACKAGE = "com.airmobis.qgroundcontrol"
QGC_APP_DESCRIPTION = "Airmobis QGroundControl"
QGC_APP_COPYRIGHT   = "Copyright (C) 2025 Airmobis. All rights reserved."

# Our own, custom resources
# Not yet used
RESOURCES += \
    $$PWD/custom.qrc
message(*** USING custom.qrc from $$PWD ***)

QML_IMPORT_PATH += \
   $$PWD/src

INCLUDEPATH += \
    $$PWD/src \

SOURCES += \
    $$PWD/src/CustomPlugin.cc \
    $$PWD/src/GeoWork.cc \
    $$PWD/src/GeoWork_qmlinit.cc \
    $$PWD/src/ColorPicker.cc \

HEADERS += \
    $$PWD/src/CustomPlugin.h \
    $$PWD/src/GeoWork.h \
    $$PWD/src/ColorPicker.h \

QT += network

LIBS += -L /usr/local/lib/libcrypto.a -lcrypto -L /usr/local/lib/libssl.a -lssl
CONFIG += staticlib
