#pragma once

#include <HerelinkOptions.h>

#include <QGCApplication.h>
#include <QGCCorePlugin.h>
#include <QGCLoggingCategory.h>

#include <QObject>

Q_DECLARE_LOGGING_CATEGORY(HerelinkCorePluginLog)

class CustomPlugin : public QGCCorePlugin {
    Q_OBJECT

    Q_PROPERTY(bool isHerelink READ isHerelink CONSTANT)

public:
    explicit CustomPlugin(QObject* parent = nullptr);

    static QGCCorePlugin* instance();
    QGCOptions*           options() final;

    constexpr bool isHerelink(void) const {
        return true;
    }

    bool overrideSettingsGroupVisibility(QString name);
    bool adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData) override;
    void factValueGridCreateDefaultSettings();

private slots:
    void activeVehicleChanged(Vehicle* activeVehicle);

private:
    HerelinkOptions _herelinkOptions;
};
