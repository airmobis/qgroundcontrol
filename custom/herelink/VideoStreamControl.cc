#include <QDebug>

#include "LinkInterface.h"
#include "MAVLinkProtocol.h"
#include "QGCApplication.h"
#include "QGCLoggingCategory.h"
#include "SettingsManager.h"
#include "VideoStreamControl.h"

QGC_LOGGING_CATEGORY(VideoStreamControlLog, "VideoStreamControlLog")

VideoStreamControl::VideoStreamControl()
    : QObject()
    , _systemId(-1)
    , _linkInterface(NULL)
    , _cameraServiceUid(0)
    , _cameraCount(0)
    , _settingInProgress(false)
{
    _mavlinkProtocol = MAVLinkProtocol::instance();
    connect(_mavlinkProtocol, &MAVLinkProtocol::messageReceived, this, &VideoStreamControl::_mavlinkMessageReceived);

    _videoSettings = SettingsManager::instance()->videoSettings();
    _cameraIdSetting = _videoSettings->cameraId()->rawValue().toUInt();
    _cameraInfoReceived = false;
    _currentHdmiInput = 0;  // Default to HDMI 1

    connect(_videoSettings->cameraId(), &Fact::rawValueChanged, this, &VideoStreamControl::_cameraIdChanged);
    //connect(&_infoRequestTimer, &QTimer::timeout, this, &VideoStreamControl::_requestVideoStreamInfo);
    //_infoRequestTimer.setInterval(1000);
}

VideoStreamControl::~VideoStreamControl()
{

}

void VideoStreamControl::_mavlinkMessageReceived(LinkInterface* link, mavlink_message_t message)
{
    if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT && message.compid == MAV_COMP_ID_CAMERA) {
        _handleHeartbeatInfo(link, message);
    } else if (message.msgid == MAVLINK_MSG_ID_VIDEO_STREAM_INFORMATION && message.compid == MAV_COMP_ID_CAMERA) {
        _handleVideoStreamInformation(message);
    }
}


void VideoStreamControl::_cameraIdChanged()
{
    qCDebug(VideoStreamControlLog) << "Camera ID changed to:" << _videoSettings->cameraId()->rawValue().toUInt();
    _setCameraIdLockUi(true);
}

void VideoStreamControl::_requestVideoStreamInfo()
{
    if (_linkInterface == NULL) {
        return;
    }
    qCDebug(VideoStreamControlLog) << "Requesting video stream information from " << (int)_mavlinkProtocol->getSystemId() << "/" << (int)_mavlinkProtocol->getComponentId() << " to " << (int)_systemId << "/" << (int)MAV_COMP_ID_CAMERA;
    mavlink_message_t msg;
    mavlink_msg_command_long_pack(_mavlinkProtocol->getSystemId(), _mavlinkProtocol->getComponentId(), &msg,
                                      _systemId, MAV_COMP_ID_CAMERA,
                                      MAV_CMD_REQUEST_VIDEO_STREAM_INFORMATION, 0, 0, 0, 0, 0, 0, 0, 0);
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &msg);

    _linkInterface->writeBytesThreadSafe((const char*)buffer, len);
}

void VideoStreamControl::_handleVideoStreamInformation(mavlink_message_t& message)
{
    mavlink_video_stream_information_t streamInfo;
    mavlink_msg_video_stream_information_decode(&message, &streamInfo);

    qWarning() << "VideoStreamControl: Received video stream information: stream_id=" << streamInfo.stream_id
               << "count=" << streamInfo.count;
    qCDebug(VideoStreamControlLog) << "Received video stream information: stream_id=" << streamInfo.stream_id
                                   << "count=" << streamInfo.count;

    // Convert stream_id from 1-based to 0-based for QGC camera setting
    uint32_t cameraIdZeroBased = (streamInfo.stream_id > 0) ? streamInfo.stream_id - 1 : 0;

    // Check if we need to start streaming based on state
    bool shouldStartStreaming = false;

    if (!_cameraInfoReceived) {
        // Case 1: Never run before, start current stream
        shouldStartStreaming = true;
        _cameraInfoReceived = true;
        emit cameraInfoReceivedChanged();
        qCDebug(VideoStreamControlLog) << "First camera information received, will start streaming";
    } else if (cameraIdZeroBased != _cameraIdSetting) {
        // Case 2: Run before but camera changed, start new stream
        shouldStartStreaming = true;
        qCDebug(VideoStreamControlLog) << "Camera changed, will restart streaming";
    }

    // Update current HDMI input and emit signal if it changed
    if (_currentHdmiInput != cameraIdZeroBased) {
        _currentHdmiInput = cameraIdZeroBased;
        emit currentHdmiInputChanged();
        qCDebug(VideoStreamControlLog) << "HDMI input changed to:" << (_currentHdmiInput == 0 ? "HDMI 1" : "HDMI 2");
    }

    // Always log current state for debugging button issues
    qWarning() << "VideoStreamControl: Current HDMI input =" << _currentHdmiInput << "(" << (_currentHdmiInput == 0 ? "HDMI 1" : "HDMI 2") << ")";

    // Update our camera setting to match the air unit's current camera
    if (cameraIdZeroBased != _cameraIdSetting) {
        qCDebug(VideoStreamControlLog) << "Air unit stream ID" << streamInfo.stream_id
                                       << "(0-based:" << cameraIdZeroBased << ") differs from QGC setting" << _cameraIdSetting;

        // Temporarily disconnect the signal to avoid triggering another change
        disconnect(_videoSettings->cameraId(), &Fact::rawValueChanged, this, &VideoStreamControl::_cameraIdChanged);

        // Update QGC's setting to match air unit (convert to 0-based)
        _videoSettings->cameraId()->setRawValue(cameraIdZeroBased);
        _cameraIdSetting = cameraIdZeroBased;

        // Reconnect the signal
        connect(_videoSettings->cameraId(), &Fact::rawValueChanged, this, &VideoStreamControl::_cameraIdChanged);

        qCDebug(VideoStreamControlLog) << "Synchronized QGC camera setting to" << cameraIdZeroBased;

        // Only restart stream if this sync represents an actual change we need to act on
        // (not just initial sync - the stream should already be correct)
        qWarning() << "VideoStreamControl: Camera synchronized but not restarting stream - should already be on correct input";
        emit videoNeedsReset(); // Removed - don't restart unnecessarily
    }

    // Start streaming if needed
    if (shouldStartStreaming) {
        //_startVideoStreaming();
    }

    // Always unlock UI when we get camera info - the buttons will show correct state
    if (_settingInProgress) {
        qCDebug(VideoStreamControlLog) << "Unlocking UI after receiving camera info";
        _setSettingInProgress(false);
    }
}

void VideoStreamControl::_handleHeartbeatInfo(LinkInterface* link, mavlink_message_t& message)
{
    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(&message, &heartbeat);

    if (message.sysid == _systemId) {
        if (heartbeat.custom_mode == _cameraServiceUid) {
            return;
        } else {
            // customMode is a uid, the change means remote peer reset
            // need to restart video streaming
            qCDebug(VideoStreamControlLog) << "remote peer reset";
            _systemId = 0;
        }
    }

    qCDebug(VideoStreamControlLog) << "First camera heartbeat:" << message.sysid << heartbeat.system_status << heartbeat.custom_mode;

    _systemId = message.sysid;
    _cameraServiceUid = heartbeat.custom_mode;
     // customMode 32bits: bits 25-31: camera count, bits 16-24: timestamp, bits 0-15 remote peer pid
    _cameraCount = _cameraServiceUid >> 24;
    qCDebug(VideoStreamControlLog) << "Camera found uid:" << _cameraServiceUid << "count:" << _cameraCount;

    _linkInterface = link;

    // Start periodic video stream information requests
    //if (!_infoRequestTimer.isActive()) {
    //    _infoRequestTimer.start();
    //}

    _requestVideoStreamInfo();
}

void VideoStreamControl::_setCameraIdLockUi(bool lockUi)
{
    qCDebug(VideoStreamControlLog) << "_setCameraIdLockUi called with lockUi:" << lockUi << "_linkInterface:" << (_linkInterface ? "valid" : "null");

    if (_linkInterface == NULL) {
        qCDebug(VideoStreamControlLog) << "No link interface available, cannot change camera ID";
        return;
    }

    uint32_t newCameraId = _videoSettings->cameraId()->rawValue().toUInt();

    // Only proceed if the camera ID has actually changed
    if (newCameraId != _cameraIdSetting /*&& _cameraCount > 1*/) {
        qCDebug(VideoStreamControlLog) << "User changed camera ID from" << _cameraIdSetting << "to" << newCameraId;

        _cameraIdSetting = newCameraId;

        _startVideoStreaming();

        if (lockUi) {
            _setSettingInProgress(true);
        }

        QTimer::singleShot(3000, this, &VideoStreamControl::_requestVideoStreamInfo);
    } else {
        qCDebug(VideoStreamControlLog) << "Camera ID unchanged, no action needed";
    }
}

void VideoStreamControl::_startVideoStreaming() {
    if (_linkInterface == NULL) {
        return;
    }
    qCDebug(VideoStreamControlLog) << "Start Video Stream" << _systemId;
    mavlink_message_t msg;
    mavlink_msg_command_long_pack(_mavlinkProtocol->getSystemId(), _mavlinkProtocol->getComponentId(), &msg,
                                      _systemId, MAV_COMP_ID_CAMERA,
                                      MAV_CMD_VIDEO_START_STREAMING, 0, _cameraIdSetting, 0, 0, 0, 0, 0, 0);
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &msg);

    _linkInterface->writeBytesThreadSafe((const char*)buffer, len);

    //QTimer::singleShot(3000, this, &VideoStreamControl::videoNeedsReset);
}

void VideoStreamControl::_setSettingInProgress(bool inProgress)
{
    if (inProgress) {
        qCDebug(VideoStreamControlLog) << "Lock UI for setting camera";
    } else {
        qCDebug(VideoStreamControlLog) << "Unlock UI after setting camera";
    }

    _settingInProgress = inProgress;
    emit settingInProgressChanged();
    return;
}
