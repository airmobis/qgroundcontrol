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
    _currentResolution = 1;  // Default to 1080p

    connect(_videoSettings->cameraId(), &Fact::rawValueChanged, this, &VideoStreamControl::_cameraIdChanged);
    connect(_videoSettings->resolution(), &Fact::rawValueChanged, this, &VideoStreamControl::_resolutionChanged);
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
    } else if (message.msgid == MAVLINK_MSG_ID_PARAM_EXT_ACK && message.compid == MAV_COMP_ID_CAMERA) {
        mavlink_param_ext_ack_t ack;
        mavlink_msg_param_ext_ack_decode(&message, &ack);
        qCDebug(VideoStreamControlLog) << "VideoStreamControl: PARAM_EXT_ACK received:"
                                       << "param_id=" << ack.param_id
                                       << "param_result=" << ack.param_result
                                       << "param_type=" << ack.param_type;
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
               << "count=" << streamInfo.count
               << "resolution=" << streamInfo.resolution_h << "x" << streamInfo.resolution_v;
    qCDebug(VideoStreamControlLog) << "Received video stream information: stream_id=" << streamInfo.stream_id
                                   << "count=" << streamInfo.count
                                   << "resolution=" << streamInfo.resolution_h << "x" << streamInfo.resolution_v;

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
    // Note: For user-initiated HDMI switches, _currentHdmiInput is already updated
    // in _setCameraIdLockUi and videoNeedsReset is already emitted there.
    // This handles cases where the air unit reports a different HDMI than expected.
    bool hdmiChanged = false;
    if (_currentHdmiInput != cameraIdZeroBased) {
        qCDebug(VideoStreamControlLog) << "HDMI input mismatch: expected" << _currentHdmiInput << "got" << cameraIdZeroBased;
        _currentHdmiInput = cameraIdZeroBased;
        hdmiChanged = true;
        emit currentHdmiInputChanged();
        qCDebug(VideoStreamControlLog) << "HDMI input changed to:" << (_currentHdmiInput == 0 ? "HDMI 1" : "HDMI 2");
    }

    // Detect and update current resolution from stream info
    uint8_t detectedResolution = 1; // Default to 1080p
    if (streamInfo.resolution_v == 720) {
        detectedResolution = 0; // 720p
    } else if (streamInfo.resolution_v == 1080) {
        detectedResolution = 1; // 1080p
    }

    bool resolutionChanged = false;
    if (_currentResolution != detectedResolution) {
        qCDebug(VideoStreamControlLog) << "Resolution changed from"
                                       << (_currentResolution == 0 ? "720p" : "1080p") << "to"
                                       << (detectedResolution == 0 ? "720p" : "1080p");
        _currentResolution = detectedResolution;
        resolutionChanged = true;
        emit currentResolutionChanged();

        // Synchronize UI setting with air unit's resolution
        disconnect(_videoSettings->resolution(), &Fact::rawValueChanged, this, &VideoStreamControl::_resolutionChanged);
        _videoSettings->resolution()->setRawValue(detectedResolution);
        connect(_videoSettings->resolution(), &Fact::rawValueChanged, this, &VideoStreamControl::_resolutionChanged);
    }

    // Handle video restarts based on what changed.
    // Don't restart on first connection (shouldStartStreaming) - pipeline is already starting.
    // Only restart for HDMI changes - resolution changes are handled dynamically by GStreamer.
    if (!shouldStartStreaming && hdmiChanged) {
        qCDebug(VideoStreamControlLog) << "Requesting full video restart for HDMI change";
        emit videoNeedsReset();
    }

    qCDebug(VideoStreamControlLog) << "Current HDMI input =" << _currentHdmiInput << "(" << (_currentHdmiInput == 0 ? "HDMI 1" : "HDMI 2") << ")"
                                   << "Resolution =" << (detectedResolution == 0 ? "720p" : "1080p");

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
        // Don't emit videoNeedsReset() here - it causes restart loops
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
        _currentHdmiInput = newCameraId;
        emit currentHdmiInputChanged();

        _startVideoStreaming();

        if (lockUi) {
            _setSettingInProgress(true);
        }

        // Trigger video reset immediately - don't wait for VIDEO_STREAM_INFORMATION
        // The air unit takes 20+ seconds to respond, so we restart the pipeline now
        // and let GStreamer's retry mechanism reconnect when the stream is ready.
        qCDebug(VideoStreamControlLog) << "Triggering immediate video reset for HDMI change";
        emit videoNeedsReset();

        // Still request stream info to confirm the change eventually
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

void VideoStreamControl::_resolutionChanged()
{
    uint32_t newResolution = _videoSettings->resolution()->rawValue().toUInt();
    qCDebug(VideoStreamControlLog) << "Resolution changed to:" << (newResolution == 0 ? "720p" : "1080p");

    if (_linkInterface == NULL) {
        qCDebug(VideoStreamControlLog) << "No link interface available, cannot change resolution";
        return;
    }

    // Only proceed if the resolution has actually changed
    if (newResolution != _currentResolution) {
        qCDebug(VideoStreamControlLog) << "User changed resolution from" << (_currentResolution == 0 ? "720p" : "1080p")
                                       << "to" << (newResolution == 0 ? "720p" : "1080p");

        // Determine parameter name based on current camera ID
        const char* param_id = (_cameraIdSetting == 0) ? "VideoRes0" : "VideoRes1";

        // Parameter value: "1" for 1080p, "0" for 720p
        const char* param_value = (newResolution == 1) ? "1" : "0";

        qCDebug(VideoStreamControlLog) << "Sending PARAM_EXT_SET:" << param_id << "=" << param_value;

        // Send PARAM_EXT_SET message
        mavlink_message_t msg;
        mavlink_param_ext_set_t param_set;

        memset(&param_set, 0, sizeof(mavlink_param_ext_set_t));
        param_set.target_system = _systemId;
        param_set.target_component = MAV_COMP_ID_CAMERA;
        strncpy(param_set.param_id, param_id, sizeof(param_set.param_id) - 1);
        strncpy(param_set.param_value, param_value, sizeof(param_set.param_value) - 1);
        param_set.param_type = MAV_PARAM_EXT_TYPE_UINT8;

        mavlink_msg_param_ext_set_encode(_mavlinkProtocol->getSystemId(), _mavlinkProtocol->getComponentId(), &msg, &param_set);

        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        int len = mavlink_msg_to_send_buffer(buffer, &msg);
        _linkInterface->writeBytesThreadSafe((const char*)buffer, len);

        // Update _currentResolution optimistically - we can't wait for VIDEO_STREAM_INFORMATION
        // because it takes too long (20+ seconds) and we need to allow subsequent changes.
        _currentResolution = newResolution;
        emit currentResolutionChanged();

        // Restart decoding after a delay to let the air unit encoder change resolution.
        // We use decodingNeedsRestart (lighter-weight) instead of videoNeedsReset (full RTSP restart).
        // The delay allows the encoder to output new resolution frames before we restart.
        QTimer::singleShot(1000, this, [this]() {
            qCDebug(VideoStreamControlLog) << "Triggering decoder restart for resolution change";
            emit decodingNeedsRestart();
        });
    } else {
        qCDebug(VideoStreamControlLog) << "Resolution unchanged, no action needed";
    }
}
