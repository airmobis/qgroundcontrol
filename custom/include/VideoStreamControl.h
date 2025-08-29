#pragma once

#include <LinkInterface.h>
#include <MAVLinkProtocol.h>

#include <Fact.h>

#include <QObject>

Q_DECLARE_LOGGING_CATEGORY(VideoStreamControlLog)

class VideoStreamControl : public QObject {
    Q_OBJECT

public:
    VideoStreamControl();

    Q_PROPERTY(bool settingInProgress READ settingInProgress NOTIFY settingInProgressChanged)
    bool settingInProgress() const;

signals:
    void settingInProgressChanged();
    void videoNeedsReset();

private slots:
    void mavlinkMessageReceived(LinkInterface* link, mavlink_message_t message);
    void settingInProgressTimeout();
    void cameraIdChanged();

private:
    int m_systemId;

    LinkInterface* m_linkInterface;
    Fact*          m_videoSource;

    QTimer m_settingInProgressTimer;

    uint32_t
        m_cameraServiceUid,
        m_cameraCount,
        m_cameraIdSetting;

    bool m_settingInProgress;

    void handleHeartbeatInfo(LinkInterface* link, mavlink_message_t& message);
    void setCameraId();
    void setCameraIdLockUi(bool lockUi);
    void startVideoStreaming();
    void setSettingInProgress(bool inProgress);
};
