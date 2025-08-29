#pragma once

#include <HerelinkOptions.h>

#include <QGCApplication.h>
#include <QGCCorePlugin.h>
#include <QGCLoggingCategory.h>

#include <QObject>

Q_DECLARE_LOGGING_CATEGORY(HerelinkCorePluginLog)

class CustomPlugin : public QGCCorePlugin {
    Q_OBJECT

public:
    explicit CustomPlugin(QGCApplication* app);

    Q_PROPERTY(bool isHerelink READ isHerelink CONSTANT)

    constexpr bool isHerelink(void) const {
        return true;
    }

    // Overrides QGCCorePlugin.
    QGCOptions* options(void) override;

    bool overrideSettingsGroupVisibility(QString name);
    bool adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData) override;
    void factValueGridCreateDefaultSettings();

private slots:
    void activeVehicleChanged(Vehicle* activeVehicle);

private:
    HerelinkOptions* m_herelinkOptions;
};
