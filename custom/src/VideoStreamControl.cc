#include "VideoStreamControl.h"

#include "SettingsManager.h"
#include "VideoSettings.h"

#include "QGCLoggingCategory.h"
#include <QDebug>

QGC_LOGGING_CATEGORY(VideoStreamControlLog, "VideoStreamControlLog")

VideoStreamControl::VideoStreamControl()
    : m_systemId { -1 }
    , m_linkInterface { nullptr }
    , m_videoSource { SettingsManager::instance()->videoSettings()->videoSource() }
    , m_cameraServiceUid { 0 }
    , m_cameraCount { 0 }
    , m_cameraIdSetting { m_videoSource->rawValue().toUInt() }
    , m_settingInProgress { false } {
    connect(MAVLinkProtocol::instance(), &MAVLinkProtocol::messageReceived, this, &VideoStreamControl::mavlinkMessageReceived);
    connect(m_videoSource, &Fact::rawValueChanged, this, &VideoStreamControl::cameraIdChanged);
    connect(&m_settingInProgressTimer, &QTimer::timeout, this, &VideoStreamControl::settingInProgressTimeout);
}

bool VideoStreamControl::settingInProgress() const {
    return m_settingInProgress;
}

void VideoStreamControl::mavlinkMessageReceived(LinkInterface* link, mavlink_message_t message) {
    if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT && message.compid == MAV_COMP_ID_CAMERA) {
        handleHeartbeatInfo(link, message);
    }
}

void VideoStreamControl::settingInProgressTimeout() {
    qCDebug(VideoStreamControlLog) << "Timeout to setting camera, unlock UI!";
    setSettingInProgress(false);
}

void VideoStreamControl::cameraIdChanged() {
    setCameraIdLockUi(true);
}

void VideoStreamControl::handleHeartbeatInfo(LinkInterface* link, mavlink_message_t& message) {
    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(&message, &heartbeat);

    if (message.sysid == m_systemId) {
        if (heartbeat.custom_mode == m_cameraServiceUid) {
            return;
        } else {
            // customMode is a uid, the change means remote peer reset
            // need to restart video streaming
            qCDebug(VideoStreamControlLog) << "remote peer reset";
            m_systemId = 0;
        }
    }

    qCDebug(VideoStreamControlLog) << "First camera heartbeat:" << message.sysid << heartbeat.system_status << heartbeat.custom_mode;

    m_systemId         = message.sysid;
    m_cameraServiceUid = heartbeat.custom_mode;

    // customMode - 32 bits:
    //  - Bits 25-31: camera count,
    //  - Bits 16-24: timestamp,
    //  - Bits 0-15 remote peer PID.
    m_cameraCount = m_cameraServiceUid >> 24;
    qCDebug(VideoStreamControlLog) << "Camera found uid:" << m_cameraServiceUid << "count:" << m_cameraCount;

    m_linkInterface = link;

    startVideoStreaming();
}

void VideoStreamControl::setCameraId() {
    if (m_cameraCount <= 1) {
        return;
    }

    m_cameraIdSetting = m_videoSource->rawValue().toUInt();
    startVideoStreaming();
}

void VideoStreamControl::setCameraIdLockUi(bool lockUi) {
    if (m_linkInterface == nullptr) {
        return;
    }

    m_cameraIdSetting = m_videoSource->rawValue().toUInt();
    setCameraId();

    if (lockUi) {
        setSettingInProgress(true);
    }
}

void VideoStreamControl::startVideoStreaming() {
    if (m_linkInterface == nullptr) {
        return;
    }

    qCDebug(VideoStreamControlLog) << "Start Video Stream" << m_systemId;

    const MAVLinkProtocol* inst { MAVLinkProtocol::instance() };
    mavlink_message_t      msg;
    mavlink_msg_command_long_pack(inst->getSystemId(), inst->getComponentId(), &msg, m_systemId, MAV_COMP_ID_CAMERA, MAV_CMD_VIDEO_START_STREAMING, 0, m_cameraIdSetting, 0, 0, 0, 0, 0, 0);

    std::uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int          len { mavlink_msg_to_send_buffer(buffer, &msg) };

    m_linkInterface->writeBytesThreadSafe(reinterpret_cast<const char*>(buffer), len);

    emit videoNeedsReset();
}

void VideoStreamControl::setSettingInProgress(bool inProgress) {
    if (inProgress) {
        m_settingInProgressTimer.setInterval(15000);
        m_settingInProgressTimer.setSingleShot(true);
        m_settingInProgressTimer.start();
        qCDebug(VideoStreamControlLog) << "Setup timer for setting camera, and lock UI";
    } else {
        if (m_settingInProgressTimer.isActive()) {
            m_settingInProgressTimer.stop();
            qCDebug(VideoStreamControlLog) << "Done for setting camera, unlock UI and clear timer";
        }
    }

    m_settingInProgress = inProgress;

    emit settingInProgressChanged();
}

// No-ops, added to fix undefined symbols.
void VideoStreamControl::settingInProgressChanged() {
}

void VideoStreamControl::videoNeedsReset() {
}
