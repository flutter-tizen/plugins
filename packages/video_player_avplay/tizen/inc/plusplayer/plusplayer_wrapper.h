// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PLUSPLAYER_WRAPPER_H
#define FLUTTER_PLUGIN_PLUSPLAYER_WRAPPER_H

#include <stddef.h>
#include <stdint.h>
#include <tizen_error.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#define PLUS_PLAYER_EXPORT __attribute__((visibility("default")))
#define PLUSPLAYER_ERROR_CLASS TIZEN_ERROR_PLAYER | 0x20
/* This is for custom defined player error. */
#define PLUSPLAYER_CUSTOM_ERROR_CLASS TIZEN_ERROR_PLAYER | 0x1000

namespace plusplayer {

enum class DisplayType { kNone, kOverlay, kEvas, kMixer, kOverlaySyncUI };

enum class DisplayMode {
  kLetterBox,
  kOriginSize,
  kFullScreen,
  kCroppedFull,
  kOriginOrLetter,
  kDstRoi,
  kAutoAspectRatio,
  kDstRoiAutoAspectRatio,
  kMax
};

enum TrackType {
  kTrackTypeAudio = 0,
  kTrackTypeVideo,
  kTrackTypeSubtitle,
  kTrackTypeMax
};

enum PlayerType {
  kDefault,
  kDASH,
};

enum class DisplayRotation { kNone, kRotate90, kRotate180, kRotate270 };

enum class State {
  kNone, /**< Player is created, but not opened */
  kIdle, /**< Player is opened, but not prepared or player is stopped */
  kTypeFinderReady,  /**< TypeFinder prepared */
  kTrackSourceReady, /**< TrackSource prepared */
  kReady,            /**< Player is ready to play(start) */
  kPlaying,          /**< Player is playing media */
  kPaused            /**< Player is paused while playing media */
};

struct Geometry {
  /**
   * @brief  start X position of Display window. [Default = 0]
   */
  int x = 0;
  /**
   * @brief  start Y position of Display window. [Default = 0]
   */
  int y = 0;
  /**
   * @brief  Width of Display window. [Default = 1920]
   */
  int w = 1920;
  /**
   * @brief  Height of Display window. [Default = 1080]
   */
  int h = 1080;
};

struct PlayerMemento {
  uint64_t playing_time = 0;  /**< Playing time of current player */
  State state = State::kNone; /**< Player status of current player */
  DisplayMode display_mode =
      DisplayMode::kMax; /**< Display mode of current player */
  DisplayType display_type =
      DisplayType::kNone; /**< Display type of current player */
  Geometry display_area;  /**< Display area of current player */
  std::map<std::string, int>
      buffer_config;                  /**< Buffer config of current player */
  bool is_live = false;               /**< Live status of current player */
  double current_playback_rate = 1.0; /**< Playback rate of current player */
  int aspect_radio_num = 0;           /**< Aspect radio num of current player */
  int aspect_radio_den = 0;           /**< Aspect radio den of current player */
};

enum class ErrorType {
  kNone = TIZEN_ERROR_NONE,                          /**< Successful */
  kOutOfMemory = TIZEN_ERROR_OUT_OF_MEMORY,          /**< Out of memory */
  kInvalidParameter = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
  kNoSuchFile = TIZEN_ERROR_NO_SUCH_FILE, /**< No such file or directory */
  kInvalidOperation = TIZEN_ERROR_INVALID_OPERATION, /**< Invalid operation */
  kFileNoSpaceOnDevice =
      TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE, /**< No space left on the device */
  kFeatureNotSupportedOnDevice =
      TIZEN_ERROR_NOT_SUPPORTED,                 /**< Not supported */
  kSeekFailed = PLUSPLAYER_ERROR_CLASS | 0x01,   /**< Seek operation failure */
  kInvalidState = PLUSPLAYER_ERROR_CLASS | 0x02, /**< Invalid state */
  kNotSupportedFile =
      PLUSPLAYER_ERROR_CLASS | 0x03,           /**< File format not supported */
  kInvalidUri = PLUSPLAYER_ERROR_CLASS | 0x04, /**< Invalid URI */
  kSoundPolicy = PLUSPLAYER_ERROR_CLASS | 0x05, /**< Sound policy error */
  kConnectionFailed =
      PLUSPLAYER_ERROR_CLASS | 0x06, /**< Streaming connection failed */
  kVideoCaptureFailed =
      PLUSPLAYER_ERROR_CLASS | 0x07,             /**< Video capture failed */
  kDrmExpired = PLUSPLAYER_ERROR_CLASS | 0x08,   /**< Expired license */
  kDrmNoLicense = PLUSPLAYER_ERROR_CLASS | 0x09, /**< No license */
  kDrmFutureUse = PLUSPLAYER_ERROR_CLASS | 0x0a, /**< License for future use */
  kDrmNotPermitted = PLUSPLAYER_ERROR_CLASS | 0x0b, /**< Format not permitted */
  kResourceLimit = PLUSPLAYER_ERROR_CLASS | 0x0c,   /**< Resource limit */
  kPermissionDenied = TIZEN_ERROR_PERMISSION_DENIED, /**< Permission denied */
  kServiceDisconnected =
      PLUSPLAYER_ERROR_CLASS | 0x0d, /**< Socket connection lost (Since 3.0) */
  kBufferSpace =
      TIZEN_ERROR_BUFFER_SPACE, /**< No buffer space available (Since 3.0)*/
  kNotSupportedAudioCodec =
      PLUSPLAYER_ERROR_CLASS |
      0x0e,  // < Not supported audio codec but video can be played (Since 4.0)
  kNotSupportedVideoCodec =
      PLUSPLAYER_ERROR_CLASS |
      0x0f,  //< Not supported video codec but audio can be played (Since 4.0)
  kNotSupportedSubtitle =
      PLUSPLAYER_ERROR_CLASS |
      0x10, /**< Not supported subtitle format (Since 4.0) */

  // TODO(euna7.ko) Can be removed. refer to
  // http://168.219.243.246:8090/pages/viewpage.action?pageId=27269511
  kDrmInfo =
      PLUSPLAYER_CUSTOM_ERROR_CLASS | 0x05, /**< playready drm error info */
  kNotSupportedFormat = PLUSPLAYER_CUSTOM_ERROR_CLASS | 0x08,
  kStreamingPlayer = PLUSPLAYER_CUSTOM_ERROR_CLASS | 0x09,
  kDtcpFsk = PLUSPLAYER_CUSTOM_ERROR_CLASS | 0x0a,
  kPreLoadingTimeOut = PLUSPLAYER_CUSTOM_ERROR_CLASS |
                       0x0b, /**< can't finish preloading in time*/
  kNetworkError =
      PLUSPLAYER_CUSTOM_ERROR_CLASS | 0x0c, /**< for network error */
  kChannelSurfingFailed =
      PLUSPLAYER_CUSTOM_ERROR_CLASS | 0x0d, /**< for channel surfing error */

  kUnknown
};

enum class StreamingMessageType {
  kNone = 0,
  // kResolutionChanged,
  // kAdEnd,
  // kAdStart,
  // kRenderDone,
  kBitrateChange,
  // kFragmentInfo,
  kSparseTrackDetect,
  // kStreamingEvent,
  // kDrmChallengeData,
  kDrmInitData,
  // kHttpErrorCode,
  // kDrmRenewSessionData,
  kStreamEventType,
  kStreamEventData,
  kStreamSyncFlush,
  kStreamMrsUrlChanged,
  kDrmKeyRotation,
  kFragmentDownloadInfo,
  kDvrLiveLag,
  kSparseTrackData,
  kConnectionRetry,
  kConfigLowLatency,
  kCurlErrorDebugInfo,
  kParDarChange,
  kDashMPDAnchor,
  kDashRemoveStream,
  kMediaSyncCSSCII,
  kDashLiveToVod
};

enum class SourceType {
  kNone,
  kBase,
  kHttp,
  kHls,
  kDash,
  kSmooth,
  kFile,
  kMem,
  kExternalSubtitle,
  kNotFound,
  kMax
};

enum class ContentFormat {
  kNone,
  kMP4Mov,
  kMpegts,
  k3GpMov,
  kAudioMpeg,
  kAudioMpegAac,
  kMkv,
  kAvi,
  kVideoAsf,
  kAppXid3,
  kAudioOgg,
  kAudioFlac,
  kFlv,
  kVideoMpeg,
  kUnknown
};

enum class DecodedVideoFrameBufferType {
  kNone,
  kCopy,
  kReference,
  kScale,
  kManualCopy,
};
enum class RscType { kVideoRenderer };

struct MessageParam {
  std::string data;
  int size = 0;
  int code = 0;  // Error or warning code
};
struct PlayerAppInfo {
  std::string id;      /**< App id */
  std::string version; /**< App version */
  std::string type;    /**< App type. ex)"MSE", "HTML5", etc.. */
};

const int kInvalidTrackIndex = -1;

struct Track {
  int index = kInvalidTrackIndex;
  int id = 0;
  std::string mimetype;
  std::string streamtype;
  std::string container_type;
  TrackType type = kTrackTypeMax;
  std::shared_ptr<char> codec_data;
  unsigned int codec_tag = 0;
  int codec_data_len = 0;
  int width = 0;
  int height = 0;
  int maxwidth = 0;
  int maxheight = 0;
  int framerate_num = 0;
  int framerate_den = 0;
  int sample_rate = 0;
  int sample_format = 0;
  int channels = 0;
  int version = 0;
  int layer = 0;
  int bits_per_sample = 0;
  int block_align = 0;
  int bitrate = 0;
  int endianness = 1234;  // little endian : 1234 others big endian
  bool is_signed = false;
  bool active = false;
  bool use_swdecoder = false;
  std::string language_code;
  std::string subtitle_format;
  Track() {}
  Track(int _index, int _id, std::string _mimetype, std::string _streamtype,
        std::string _container_type, TrackType _type,
        std::shared_ptr<char> _codec_data, unsigned int _codec_tag,
        int _codec_data_len, int _width, int _height, int _maxwidth,
        int _maxheight, int _framerate_num, int _framerate_den,
        int _sample_rate, int _sample_format, int _channels, int _version,
        int _layer, int _bits_per_sample, int _block_align, int _bitrate,
        int _endianness, bool _is_signed, bool _active, bool _use_swdecoder,
        std::string _language_code, std::string _subtitle_format)
      : index(_index),
        id(_id),
        mimetype(_mimetype),
        streamtype(_streamtype),
        container_type(_container_type),
        type(_type),
        codec_data(_codec_data),
        codec_tag(_codec_tag),
        codec_data_len(_codec_data_len),
        width(_width),
        height(_height),
        maxwidth(_maxwidth),
        maxheight(_maxheight),
        framerate_num(_framerate_num),
        framerate_den(_framerate_den),
        sample_rate(_sample_rate),
        sample_format(_sample_format),
        channels(_channels),
        version(_version),
        layer(_layer),
        bits_per_sample(_bits_per_sample),
        block_align(_block_align),
        bitrate(_bitrate),
        endianness(_endianness),
        is_signed(_is_signed),
        active(_active),
        use_swdecoder(_use_swdecoder),
        language_code(_language_code),
        subtitle_format(_subtitle_format) {}
};

namespace drm {
using LicenseAcquiredCb = void*;
using UserData = void*;
using DrmHandle = int;

enum class Type {
  kNone = 0,
  kPlayready,
  kMarlin,
  kVerimatrix,
  kWidevineClassic,
  kSecuremedia,
  kSdrm,
  kWidevineCdm = 8,
  kMax
};

struct Property {
  Type type = Type::kNone;           // Drm type
  DrmHandle handle = 0;              // Drm handle
  bool external_decryption = false;  // External Decryption Mode
  LicenseAcquiredCb license_acquired_cb =
      nullptr;  // The cb will be invoked when license was acquired.
  UserData license_acquired_userdata =
      nullptr;  // The userdata will be sent by license_acquired_cb
};
}  // namespace drm

/**
 * @brief Enumeration for  player supported subtitle types
 */
enum class SubtitleType {
  kText,       /**< subtitle type text */
  kPicture,    /**< subtitle type picture */
  kTTMLRender, /**< subtitle type ttml */
  kInvalid     /**< unsupported subtitle type */
};

}  // namespace plusplayer

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*OnPlayerPrepared)(bool ret, void* user_data);
typedef void (*OnPlayerSeekCompleted)(void* user_data);
typedef void (*OnPlayerResourceConflicted)(void* user_data);
typedef void (*OnPlayerBuffering)(int percent, void* user_data);
typedef void (*OnPlayerCompleted)(void* user_data);
typedef void (*OnPlayerPlaying)(void* user_data);
typedef void (*OnPlayerError)(const plusplayer::ErrorType& error_code,
                              void* user_data);
typedef void (*OnPlayerErrorMessage)(const plusplayer::ErrorType& error_code,
                                     const char* error_msg, void* user_data);
typedef void (*OnPlayerAdaptiveStreamingControl)(
    const plusplayer::StreamingMessageType& type,
    const plusplayer::MessageParam& msg, void* user_data);
typedef void (*OnPlayerDrmInitData)(int* drmhandle, unsigned int len,
                                    unsigned char* psshdata,
                                    plusplayer::TrackType type,
                                    void* user_data);
typedef void (*OnPlayerClosedCaptionData)(std::unique_ptr<char[]> data,
                                          const int size, void* user_data);
typedef void (*OnPlayerCueEvent)(const char* CueData, void* userdata);
typedef void (*OnPlayerDateRangeEvent)(const char* DateRangeData,
                                       void* user_data);
typedef void (*OnPlayerStopReachEvent)(bool StopReach, void* user_data);
typedef void (*OnPlayerCueOutContEvent)(const char* CueOutContData,
                                        void* user_data);
typedef void (*OnPlayerChangeSourceDone)(bool ret, void* user_data);
typedef void (*OnPlayerStateChangedToPlaying)(void* user_data);
typedef void (*OnPlayerDrmType)(plusplayer::drm::Type drmtype, void* user_data);
typedef void (*OnPlayerSubtitleData)(char* data, const int size,
                                     const plusplayer::SubtitleType& type,
                                     const uint64_t duration, void* user_data);

struct PlusplayerListener {
  OnPlayerPrepared prepared_callback{nullptr};
  OnPlayerSeekCompleted seek_completed_callback{nullptr};
  OnPlayerResourceConflicted resource_conflicted_callback{nullptr};
  OnPlayerBuffering buffering_callback{nullptr};
  OnPlayerCompleted completed_callback{nullptr};
  OnPlayerPlaying playing_callback{nullptr};
  OnPlayerError error_callback{nullptr};
  OnPlayerErrorMessage error_message_callback{nullptr};
  OnPlayerAdaptiveStreamingControl adaptive_streaming_control_callback{nullptr};
  OnPlayerDrmInitData drm_init_data_callback{nullptr};
  OnPlayerClosedCaptionData closed_caption_data_callback{nullptr};
  OnPlayerCueEvent cue_event_callback{nullptr};
  OnPlayerDateRangeEvent data_range_event_callback{nullptr};
  OnPlayerStopReachEvent stop_reach_event_callback{nullptr};
  OnPlayerCueOutContEvent cue_out_cont_event_callback{nullptr};
  OnPlayerChangeSourceDone change_source_done_callback{nullptr};
  OnPlayerStateChangedToPlaying state_changed_to_playing_callback{nullptr};
  OnPlayerDrmType drm_type_callback{nullptr};
  OnPlayerSubtitleData subtitle_data_callback{nullptr};
};

struct Plusplayer;
typedef struct Plusplayer* PlusplayerRef;

PLUS_PLAYER_EXPORT PlusplayerRef CreatePlayer(plusplayer::PlayerType type);

PLUS_PLAYER_EXPORT bool Activate(PlusplayerRef player,
                                 const plusplayer::TrackType type);

PLUS_PLAYER_EXPORT bool Deactivate(PlusplayerRef player,
                                   const plusplayer::TrackType type);

PLUS_PLAYER_EXPORT bool SetVolume(PlusplayerRef player, int volume);

PLUS_PLAYER_EXPORT bool Open(PlusplayerRef player, const std::string& uri);

PLUS_PLAYER_EXPORT void SetAppId(PlusplayerRef player,
                                 const std::string& app_id);

PLUS_PLAYER_EXPORT void SetPrebufferMode(PlusplayerRef player,
                                         bool is_prebuffer_mode);

PLUS_PLAYER_EXPORT void SetAppInfo(PlusplayerRef player,
                                   const plusplayer::PlayerAppInfo& app_info);

PLUS_PLAYER_EXPORT bool StopSource(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool ChangeSource(
    PlusplayerRef player, const std::string& uri,
    const plusplayer::SourceType source_type,
    const plusplayer::ContentFormat format_type,
    const uint64_t time_milliseconds, const bool is_seamless);

PLUS_PLAYER_EXPORT bool Prepare(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool PrepareAsync(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool Start(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool Stop(PlusplayerRef player);

PLUS_PLAYER_EXPORT void SetDrm(PlusplayerRef player,
                               const plusplayer::drm::Property& property);

PLUS_PLAYER_EXPORT bool Pause(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool Resume(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool Close(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool Seek(PlusplayerRef player,
                             const uint64_t time_millisecond);

PLUS_PLAYER_EXPORT void SetStopPosition(PlusplayerRef player,
                                        const uint64_t time_millisecond);

PLUS_PLAYER_EXPORT bool Suspend(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool Restore(PlusplayerRef player, plusplayer::State state);

PLUS_PLAYER_EXPORT bool GetMemento(PlusplayerRef player,
                                   plusplayer::PlayerMemento* memento);

PLUS_PLAYER_EXPORT bool SetDisplay(PlusplayerRef player,
                                   const plusplayer::DisplayType& type,
                                   const uint32_t serface_id, const int x,
                                   const int y, const int w, const int h);

PLUS_PLAYER_EXPORT bool SetDisplayMode(PlusplayerRef player,
                                       const plusplayer::DisplayMode& mode);

PLUS_PLAYER_EXPORT bool SetDisplayRoi(PlusplayerRef player,
                                      const plusplayer::Geometry& roi);

PLUS_PLAYER_EXPORT bool SetDisplayRotate(
    PlusplayerRef player, const plusplayer::DisplayRotation& rotate);

PLUS_PLAYER_EXPORT bool GetDisplayRotate(PlusplayerRef player,
                                         plusplayer::DisplayRotation* rotate);

PLUS_PLAYER_EXPORT bool GetDisplayRotationSupport(PlusplayerRef player,
                                                  bool& can_rotate);

PLUS_PLAYER_EXPORT bool IsRotatableDevice(PlusplayerRef player);

PLUS_PLAYER_EXPORT bool SetDisplayVisible(PlusplayerRef player,
                                          bool is_visible);

PLUS_PLAYER_EXPORT bool SetAudioMute(PlusplayerRef player, bool is_mute);

PLUS_PLAYER_EXPORT bool SetBufferConfig(
    PlusplayerRef player, const std::pair<std::string, int>& config);

PLUS_PLAYER_EXPORT plusplayer::State GetState(PlusplayerRef player);

PLUS_PLAYER_EXPORT std::string GetTrackLanguageCode(PlusplayerRef player,
                                                    plusplayer::TrackType type,
                                                    int index);

PLUS_PLAYER_EXPORT int GetTrackCount(PlusplayerRef player,
                                     plusplayer::TrackType type);

PLUS_PLAYER_EXPORT std::vector<plusplayer::Track> GetTrackInfo(
    PlusplayerRef player);

PLUS_PLAYER_EXPORT std::vector<plusplayer::Track> GetActiveTrackInfo(
    PlusplayerRef player);

PLUS_PLAYER_EXPORT bool GetDuration(PlusplayerRef player,
                                    int64_t* duration_in_milliseconds);

PLUS_PLAYER_EXPORT bool GetPlayingTime(PlusplayerRef player,
                                       uint64_t* time_in_milliseconds);

PLUS_PLAYER_EXPORT bool SetSilentSubtitle(PlusplayerRef player, bool onoff);

PLUS_PLAYER_EXPORT bool SetPlaybackRate(PlusplayerRef player,
                                        const double speed);

PLUS_PLAYER_EXPORT bool SetPlaybackRateBySeek(PlusplayerRef player,
                                              const double rate);

PLUS_PLAYER_EXPORT void DestroyPlayer(PlusplayerRef player);

PLUS_PLAYER_EXPORT void DrmLicenseAcquiredDone(PlusplayerRef player,
                                               plusplayer::TrackType type);

PLUS_PLAYER_EXPORT void SetStreamingProperty(PlusplayerRef player,
                                             const std::string& type,
                                             const std::string& value);

PLUS_PLAYER_EXPORT std::string GetStreamingProperty(PlusplayerRef player,
                                                    const std::string& type);

PLUS_PLAYER_EXPORT bool SelectTrack(PlusplayerRef player,
                                    plusplayer::TrackType type, int index);

PLUS_PLAYER_EXPORT bool SetSubtitlePath(PlusplayerRef player,
                                        const std::string& path);

PLUS_PLAYER_EXPORT bool SetPreferredLanguage(
    PlusplayerRef player, plusplayer::TrackType type,
    const std::string& primary_language, const std::string& secondary_language,
    const std::string& tertiary_language);

PLUS_PLAYER_EXPORT void SetVideoFrameBufferType(
    PlusplayerRef player, const plusplayer::DecodedVideoFrameBufferType type);

PLUS_PLAYER_EXPORT bool GetVirtualRscId(PlusplayerRef player,
                                        const plusplayer::RscType type,
                                        int* virtual_id);

PLUS_PLAYER_EXPORT bool SetData(PlusplayerRef player, const std::string data);

PLUS_PLAYER_EXPORT bool GetData(PlusplayerRef player, std::string& data);

PLUS_PLAYER_EXPORT void RegisterListener(PlusplayerRef player,
                                         PlusplayerListener* listener,
                                         void* user_data);

PLUS_PLAYER_EXPORT void UnregisterListener(PlusplayerRef player);
};
#if defined(__cplusplus)
// extern "C"
#endif

#endif  // FLUTTER_PLUGIN_PLUSPLAYER_WRAPPER_H
