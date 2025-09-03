#include "HerelinkCorePlugin.h"

#include "AppSettings.h"
#include "AutoConnectSettings.h"
#include "InstrumentValueData.h"
#include "JoystickManager.h"
#include "MultiVehicleManager.h"
#include "QGCApplication.h"
#include "VideoSettings.h"

QGC_LOGGING_CATEGORY(HerelinkCorePluginLog, "HerelinkCorePluginLog")

Q_APPLICATION_STATIC(HerelinkCorePlugin, _customPluginInstance);

HerelinkCorePlugin::HerelinkCorePlugin(QObject* parent)
    : QGCCorePlugin { parent }
    , _herelinkOptions { this, parent } {
}

QGCCorePlugin* HerelinkCorePlugin::instance() {
    return _customPluginInstance;
}

QGCOptions* HerelinkCorePlugin::options() {
    return qobject_cast<QGCOptions*>(&_herelinkOptions);
}

bool HerelinkCorePlugin::overrideSettingsGroupVisibility(const QString& name) {
    // Hide all AutoConnect settings.
    return name != AutoConnectSettings::name;
}

bool HerelinkCorePlugin::adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData) {
    if (settingsGroup == AppSettings::settingsGroup) {
        // Default Herelink font size of 10, nice starting point.
        if (metaData.name() == AppSettings::appFontPointSizeName) {
            metaData.setRawDefaultValue(10);

            return true;
        }

        // Dark palette by default.
        if (metaData.name() == AppSettings::indoorPaletteName) {
            metaData.setRawDefaultValue(1);

            return true;
        }
    }

    if (settingsGroup == AutoConnectSettings::settingsGroup) {
        // We have to adjust the Herelink UDP autoconnect settings for the AirLink
        if (metaData.name() == AutoConnectSettings::udpListenPortName) {
            metaData.setRawDefaultValue(14551);
        } else if (metaData.name() == AutoConnectSettings::udpTargetHostIPName) {
            metaData.setRawDefaultValue(QStringLiteral("127.0.0.1"));
        } else if (metaData.name() == AutoConnectSettings::udpTargetHostPortName) {
            metaData.setRawDefaultValue(15552);
        } else {
            // Disable all the other autoconnect types
            const char* const disabledAndHiddenSettings[] {
                AutoConnectSettings::autoConnectPixhawkName,
                AutoConnectSettings::autoConnectSiKRadioName,
                AutoConnectSettings::autoConnectRTKGPSName,
                AutoConnectSettings::autoConnectLibrePilotName,
                AutoConnectSettings::autoConnectNmeaPortName,
                AutoConnectSettings::autoConnectZeroConfName,
            };

            for (const std::string_view dahs : disabledAndHiddenSettings) {
                if (metaData.name() == dahs.data()) {
                    metaData.setRawDefaultValue(false);
                }
            }
        }
    } else if (settingsGroup == VideoSettings::settingsGroup) {
        if (metaData.name() == VideoSettings::rtspTimeoutName) {
            metaData.setRawDefaultValue(60);
        } else if (metaData.name() == VideoSettings::videoSourceName) {
            metaData.setRawDefaultValue(VideoSettings::videoSourceHerelinkAirUnit);
        }
    } else if (settingsGroup == AppSettings::settingsGroup) {
        if (metaData.name() == AppSettings::androidSaveToSDCardName) {
            metaData.setRawDefaultValue(true);
        }
    }

    return true;
}

void HerelinkCorePlugin::activeVehicleChanged(Vehicle* activeVehicle) {
    if (activeVehicle == nullptr) {
        return;
    }

    QString          herelinkButtonsJoystickName = "gpio-keys";
    JoystickManager* joystickManager             = JoystickManager::instance();

    if (joystickManager->activeJoystickName() != herelinkButtonsJoystickName) {
        if (!joystickManager->setActiveJoystickName(herelinkButtonsJoystickName)) {
            qgcApp()->showAppMessage("Warning: Herelink buttton setup failed. Buttons will not work.");

            return;
        }
    }

    activeVehicle->setJoystickEnabled(true);
}
