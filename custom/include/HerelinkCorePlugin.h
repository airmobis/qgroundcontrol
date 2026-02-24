#pragma once

#include "GeoWork.h"
#include "HerelinkOptions.h"
#include "UrlInterceptor.h"

#include <QGCApplication.h>
#include <QGCCorePlugin.h>
#include <QGCLoggingCategory.h>

#include <QQmlApplicationEngine>

Q_DECLARE_LOGGING_CATEGORY(HerelinkCorePluginLog)

class HerelinkCorePlugin final : public QGCCorePlugin {
private:
    Q_OBJECT

    Q_PROPERTY(bool isHerelink READ isHerelink CONSTANT)

public:
    explicit HerelinkCorePlugin(QObject* parent = nullptr);
    virtual ~HerelinkCorePlugin() = default;

    static QGCCorePlugin* instance();
    QGCOptions*           options() final;
    void                  cleanup() final;

    QQmlApplicationEngine* createQmlApplicationEngine(QObject* parent) final;

    constexpr bool isHerelink(void) const { return true; }

    bool overrideSettingsGroupVisibility(const QString& name) final;
    bool adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData) final;

private slots:
    void activeVehicleChanged(Vehicle* activeVehicle);

private:
    HerelinkOptions m_herelinkOptions;

    QQmlApplicationEngine* m_qml;
    UrlInterceptor*        m_url;
};
