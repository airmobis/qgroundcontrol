message(STATUS "QGC: Adding Custom Plugin")

# Enable custom build
set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
    APPEND PROPERTY COMPILE_DEFINITIONS
    QGC_CUSTOM_BUILD
    CUSTOMHEADER="HerelinkCorePlugin.h"
    CUSTOMCLASS=HerelinkCorePlugin
)

if(ANDROID)
    set(CUSTOM_ANDROID_DIR "${CMAKE_SOURCE_DIR}/custom/android")
    if(EXISTS "${CUSTOM_ANDROID_DIR}")
        file(GLOB CUSTOM_ANDROID_FILES "${CUSTOM_ANDROID_DIR}/*")
        if(CUSTOM_ANDROID_FILES)
            message(STATUS "QGC: Custom Android package template found. Overlaying custom files...")
            set(DEFAULT_ANDROID_DIR "${CMAKE_SOURCE_DIR}/android")
            set(FINAL_ANDROID_DIR "${CMAKE_BINARY_DIR}/custom/android")
            file(COPY "${DEFAULT_ANDROID_DIR}/." DESTINATION "${FINAL_ANDROID_DIR}")
            file(COPY "${CUSTOM_ANDROID_DIR}/." DESTINATION "${FINAL_ANDROID_DIR}")
            set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                            "${DEFAULT_ANDROID_DIR}/"
                            "${CUSTOM_ANDROID_DIR}/"
                        )
            set(QGC_ANDROID_PACKAGE_SOURCE_DIR "${FINAL_ANDROID_DIR}" CACHE PATH "Path to a custom Android package template" FORCE)
            message(STATUS "QGC: Android package template path will be set to: ${QGC_ANDROID_PACKAGE_SOURCE_DIR}")
        else()
            message(STATUS "QGC: Custom Android package template empty. Using default.")
        endif()
    else()
        message(STATUS "QGC: No custom Android package template found. Using default.")
    endif()
endif()

# Our own, custom resources
list(APPEND CUSTOM_RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/custom.qrc
)

set(
    QGC_RESOURCES ${QGC_RESOURCES} ${CUSTOM_RESOURCES}
    CACHE STRING "Paths to .qrc resources" FORCE
)

set(
    QML_IMPORT_PATH ${QML_IMPORT_PATH} "${CMAKE_CURRENT_LIST_DIR}/res"
    CACHE STRING "Extra QML import paths" FORCE
)

set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/res/FlyViewCustomLayer.qml PROPERTIES
    QT_RESOURCE_ALIAS FlyViewCustomLayer.qml
)

set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/res/GeoWorkSettingsPanel.qml PROPERTIES
    QT_RESOURCE_ALIAS GeoWorkSettingsPanel.qml
)

qt_add_qml_module(CustomModule
    URI Custom
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES
        ${CMAKE_CURRENT_LIST_DIR}/res/FlyViewCustomLayer.qml
        ${CMAKE_CURRENT_LIST_DIR}/res/GeoWorkSettingsPanel.qml
    NO_PLUGIN
)

# WARNING Headers need to be included as well in order for
# Qt's MOC to do its job correctly--do not remove!
set(CUSTOM_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/include/HerelinkCorePlugin.h
    ${CMAKE_CURRENT_LIST_DIR}/include/HerelinkOptions.h
    ${CMAKE_CURRENT_LIST_DIR}/include/GeoWork.h
    ${CMAKE_CURRENT_LIST_DIR}/include/UrlInterceptor.h
    ${CMAKE_CURRENT_LIST_DIR}/include/VideoStreamControl.h

    ${CMAKE_CURRENT_LIST_DIR}/src/HerelinkCorePlugin.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/HerelinkOptions.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/GeoWork.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/GeoWork_qmlinit.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/UrlInterceptor.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/VideoStreamControl.cc

    CACHE INTERNAL "" FORCE
)

# Explicitly link QML module, needed by Qt 6.6.3
# TODO Remove when support for this version is dropped
set(CUSTOM_LIBRARIES
    CustomModule
    CACHE INTERNAL "[Custom] Libraries to link" FORCE
)

set(CUSTOM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/include
    CACHE INTERNAL "[Custom] Directories to include" FORCE
)
