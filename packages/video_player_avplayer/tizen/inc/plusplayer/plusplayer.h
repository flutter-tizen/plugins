/**
 * @file           plusplayer.h
 * @interfacetype  module
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        1.0
 * @SDK_Support    N
 * @see            defaultplayer.h
 *
 * Copyright (c) 2018 Samsung Electronics Co., Ltd All Rights Reserved
 * PROPRIETARY/CONFIDENTIAL
 * This software is the confidential and proprietary
 * information of SAMSUNG ELECTRONICS ("Confidential Information"). You shall
 * not disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into with
 * SAMSUNG ELECTRONICS. SAMSUNG make no representations or warranties about the
 * suitability of the software, either express or implied, including but not
 * limited to the implied warranties of merchantability, fitness for a
 * particular purpose, or non-infringement. SAMSUNG shall not be liable for any
 * damages suffered by licensee as a result of using, modifying or distributing
 * this software or its derivatives.
 */
#ifndef __PLUSPLAYER_PLUSPLAYER__H__
#define __PLUSPLAYER_PLUSPLAYER__H__

#include <boost/core/noncopyable.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

// plusplayer should disable mix feature temporarily.
//#ifndef IS_AUDIO_PRODUCT
//#include "mixer/mixer.h"
//#endif

#include "plusplayer/appinfo.h"
#include "plusplayer/drm.h"
#include "plusplayer/eventlistener.h"
#include "plusplayer/track.h"
#include "plusplayer/types/buffer.h"
#include "plusplayer/types/display.h"
#include "plusplayer/types/picturequality.h"
#include "plusplayer/types/resource.h"
#include "plusplayer/types/source.h"

//#ifndef PLUS_PLAYER_ENABLE_AI_ABR
//#define PLUS_PLAYER_ENABLE_AI_ABR
//#endif

namespace plusplayer {

/**
 * @brief Enumeration for media player state.
 */
enum class State {
  kNone, /**< Player is created, but not opened */
  kIdle, /**< Player is opened, but not prepared or player is stopped */
  kTypeFinderReady,  /**< TypeFinder prepared */
  kTrackSourceReady, /**< TrackSource prepared */
  kReady,            /**< Player is ready to play(start) */
  kPlaying,          /**< Player is playing media */
  kPaused            /**< Player is paused while playing media */
};
/**
 * @brief Player information of current player to be restored
 * @brief To support suspend/restore
 */
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
};

/**
 * @brief   Enumerations for player codec type
 */
enum PlayerCodecType {
  /**
   * @description   hardware codec can only be selected, default type
   */
  kPlayerCodecTypeHW,
  /**
   * @description   software codec can only be selected.
   */
  kPlayerCodecTypeSW,
};

/**
 * @brief  Enumerations for video resource type
 */
enum VideoResourceType {
  /**
   * @description   video resource(main), default type
   */
  kVideoResource,
  /**
   * @description   sub video resource.
   */
  kVideoResourceSub,
};

/**
 * @brief  Enumerations for video resource type
 */
enum PlayerType {
  /**
   * @description   default player type
   */
  kDefault,
  /**
   * @description   for dash extened player.
   */
  kDASH,
};

/**
 * @brief Class Plusplayer
 */
/**
 * @brief   [Mandatory] Write Class Outline.
 * @remark  [Optional] Write Detailed Explanation of Class.
 * @see     [Optional] Related Information
 */
class PlusPlayer : private boost::noncopyable {
 public:
  using Ptr = std::unique_ptr<PlusPlayer>;
  /**
   * @brief     Create a plusplayer object
   * @remarks   You must use this to get plusplayer object
   * @return    Player object (unique_ptr<PlusPlayer>)
   * @code
   * #include <plusplayer.h>
   *
   * class PlusPlayerEventListener : public plusplayer::EventListener {
   * public:
   *  void OnPrepareDone(bool ret, UserData userdata) {
   *    // do something
   *    prepared = true;
   *  }
   * };
   * PlusPlayerEventListener listener;  // for RegisterListener()
   * std::string url {"http://xxxx.mp4"};  // for Open()
   *
   * // call sequence
   * std::unique_ptr<PlusPlayer> plusplayer = plusplayer::PlusPlayer::Create();
   * plusplayer->RegisterListener(listener)
   * plusplayer->Open(url);
   * if(haveSurfaceId == true) {
   *   plusplayer_->SetDisplay(plusplayer::DisplayType::kOverlay, display_id, x,
   * y, w, h) } else { Evas_Object* obj = elm_win_add(NULL, "player",
   * ELM_WIN_BASIC);  // for SetDisplay() evas_object_move(obj, 0, 0);
   *   evas_object_resize(obj, 1920, 1080);
   *   evas_object_show(obj);
   *   plusplayer->SetDisplay(plusplayer::DisplayType::kOverlay, obj);
   * }
   *
   * if(isResumePlayback == true) {  // (Optional)
   *   plusplayer->Seek(10000);  // to start from 10 sec
   * }
   *
   * if(doSomethingDuringPrepare == true) {
   *   plusplayer->PrepareAsync();
   *   // do something. you should call Start() after OnPrepareDone() is called.
   * } else {
   *   plusplayer->Prepare();
   *   prepared = true;
   * }
   *
   * while(true) {
   *   if(prepared == true) {
   *     plusplayer->Start();
   *     break;
   *   }
   *   // waiting until player is prepared.
   * }
   * // playing a content...
   *
   * if(needtoDoOptionals == true) {  // (Optional)
   *   plusplayer->Seek(60000);  // to seek to 60sec
   *   // playing a content from 60 sec...
   *
   *   plusplayer->Pause();  // to pause
   *   plusplayer->Resume();  // to resume
   *   plusplayer->SetPlaySpeed(2.0);  // to set play speed
   *   // 2x playback...
   * }
   *
   * plusplayer->Stop();
   * plusplayer->Close();
   * @endcode
   */
  static Ptr Create();
  static Ptr Create(PlayerType type);

 public:
  virtual ~PlusPlayer() {}
  /**
   * @brief     Make player get ready to set playback configurations of URI
   * @remarks   General call sequence to start playback:
   *             Open() -> Prepare() -> Start() -> Stop() -> Close()
   * @param     [in] uri : The content location, such as the file path,
   *            the URI of the HTTP or RTSP stream you want to play
   * @pre       Player state must be kNone
   * @post      The player state will be State::kIdle
   * @return    @c True on success, otherwise @c False
   * @see       Close()
   */
  virtual bool Open(const std::string& uri) { return false; }

  /**
   * @brief     Make player get ready to set playback configurations of
   * 2nd/nextURI
   * @remarks   General call sequence to start playback:
   *        Open() -> OpenNext() -> ChangeSource() -> Start() -> Stop() ->
   * Close()
   * @param     [in] uri : The content location, such as the file path,
   *            the URI of the HTTP or RTSP stream you want to play
   * @pre       Player state must larger than kReady (trackrender actived)
   * @post      The player state will be same state before it called
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::Open()
   */
  virtual bool OpenNext(const std::string& uri) { return false; }

  /**
   * @brief     Release all the player resources
   * @pre       The player state must be one of
   *            State::kIdle or State::kNone
   * @post      The player state will be kNone
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::Open()
   */
  virtual bool Close() { return false; }
  /**
   * @brief     sets app_id to resource manager
   * @param     [in] app_id : application id
   * @pre       The player state must be set to #State::kIdle
   * @post      None
   * @exception None
   */
  virtual void SetAppId(const std::string& app_id) { return; }
  /**
   * @brief     SetAppInfo
   * @remarks   Set app_info to player
   * @param     [in] app_info : application id, version, type
   * @pre       The player state must be set to #State::kIdle
   * @return    None
   */
  virtual void SetAppInfo(const PlayerAppInfo& app_info) { return; }
  /**
   * @brief     Set prebuffer mode
   * @details   Player doesn't acquire h/w resources from resourcemanager
   *              when it is prepared if `is_prebuffer_mode` set true
   * @remarks   Default value is false if user doesn't set.
   * @param     [in] is_prebuffer_mode : if you want to use prebuffer mode, set
   * it to @c true
   * @pre       The player state must be #State::kIdle
   * @return    @c True on success, otherwise @c False
   */
  virtual void SetPrebufferMode(bool is_prebuffer_mode) { return; }
  /**
  * @brief     Stop source
  * @details   Stop current source to change source.
               User can set streaming properties for next source after stopping
  the current source.
  * @remarks  stopped source will be removed internally so it can't be reused.
  * @pre       The player must be one of #State::kReady or #State::kPlaying or
  #State::kPaused
  * @post      In case of success the player state will be #State::kIdle
  *            otherwise state will be not changed.
               post state is #State::kIdle.
               But Open(), SetDrm(), SetPrebufferMode(), Prepare(),
  PrepareAsync(), Seek() are NOT available on #State::kIdle which is changed by
  StopSource().
  * @return    @c True on success, otherwise @c False
  * @see       PlusPlayer::ChangeSource()
  */
  virtual bool StopSource() { return false; }
  /**
  * @brief     Change source
  * @details   delete old source and then add new source for playing new stream
  * @remarks   EventListener::OnChangeSourceDone() will be called when
  ChangeSource is finished It is able to use in the following cases due to HW
  restrictions Restriction conditions : same codec, samplerate, channel,
  resolution(UHD or not)
  * @param     [in] uri : the content location, such as the URI of stream you
  want to play
  * @param     [in] source_type : streaming type
  * @param     [in] format_type : content format type
  * @param     [in] time_millisecond : the absolute position(playingtime) of the
  stream in milliseconds
  * @param     [in] is_seamless : the setting to display mute or not between the
  stream
  * @pre
  * @return    @c True on success, otherwise @c False
  */
  virtual bool ChangeSource(const std::string& uri,
                            const SourceType source_type,
                            const ContentFormat format_type,
                            const uint64_t time_milliseconds,
                            const bool is_seamless) {
    return false;
  }
  /**
   * @brief     Prepare the player for playback
   * @details   Make player get ready to start playback \n
   *            Prepare at least one decoded frame that can be displayed \n
   *            Get AV h/w resources if PlusPlayer::SetPrebufferMode() is false
   * \n One frame will be displayed when prepare is done unless
   * PlusPlayer::SetDisplayVisible() is set to @c false
   * @pre       The player state must be set to #State::kIdle
   * @post      The player state will be #State::kReady
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::SetDisplayVisible()
   */
  virtual bool Prepare() { return false; }
  /**
   * @brief     Prepare the player for playback, asynchronously.
   * @remarks   EventListener::OnPrepareDone() will be called when prepare is
   * finished
   * @pre       The player state must be set to #State::kIdle
   * @post      The player state will be #State::kReady
   * @return    @c true if async task is correctly started
   * @see       PlusPlayer::Open() \n
   *            PlusPlayer::Stop()
   */
  virtual bool PrepareAsync() { return false; }
  /**
   * @brief     Start playback
   * @pre       The player state should be #State::kReady
   * @post      The player state will be #State::kPlaying
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::Open()  \n
   *            PlusPlayer::Prepare()  \n
   *            PlusPlayer::PrepareAsync() \n
   *            PlusPlayer::Stop() \n
   *            PlusPlayer::Close()
   */
  virtual bool Start() { return false; }
  /**
   * @brief     Stop playing media content
   * @remarks   PlusPlayer::Close() must be called once after player is stopped
   * @pre       The player state must be all of #State except #State::kNone
   * @post      The player state will be #State::kIdle
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::Open() \n
   *            PlusPlayer::Prepare() \n
   *            PlusPlayer::PrepareAsync() \n
   *            PlusPlayer::Start() \n
   *            PlusPlayer::Pause() \n
   *            PlusPlayer::Resume() \n
   *            PlusPlayer::Close()
   */
  virtual bool Stop() { return false; }
  /**
   * @brief     Pause playing media content
   * @remarks   You can resume playback by using PlusPlayer::Resume()
   * @pre       The player state must be one of #State::kReady or
   * #State::kPlaying or #State::kPaused
   * @post      The player state will be #State::kPaused
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::Start() \n
   *            PlusPlayer::Resume() \n
   */
  virtual bool Pause() { return false; }
  /**
   * @brief     Resume a media content
   * @pre       The player state must be one of #State::kPlaying or
   * #State::kPaused
   * @post      The player state will be #plusplayer::State::kPlaying
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::Start()
   * @see       PlusPlayer::Pause()
   */
  virtual bool Resume() { return false; }
  /**
   * @brief     Seek for playback, asynchronously.
   * @remarks   In case of non-seekable content, it will return @c False \n
   *            If application ignore this error,
   *             player will keep playing without changing play position. \n
   *            EventListener::OnSeekDone() will be called if seek operation is
   * finished \n Seek result can be succeeded or not at this moment. \n
   *            Resumeplay : Seek() can be called in #State::kIdle \n
   *            Call sequence of resumeplay : Open() -> Seek() with
   *            resume_position -> Prepare() -> Start()
   * @param     [in] time_millisecond : the absolute position(playingtime) of
   * the stream in milliseconds
   * @pre       The player state must be one of #State::kReady,
   *            #State::kPlaying or #State::kPaused or #State::kIdle
   * @post      None
   * @exception None
   * @return    @c True if seek operation is started without any problem
   * otherwise @c False
   * @see       EventListener::OnSeekDone()
   */
  virtual bool Seek(const uint64_t time_millisecond) { return false; }
  /**
   * @brief     Set stop point for playback.
   * @remarks   Only support hls. If stop position is set, media playeback will
   * stop at time_millisecond instead of eos.
   * @param     [in] time_millisecond : the absolute position(playingtime) of
   * the stream in milliseconds
   * @pre       The player state must be one of #State::kTrackSourceReady,
   * #State::kReady, #State::kPlaying or #State::kPaused
   * @post      None
   * @exception None
   * @see       EventListener::OnStopReachEvent()
   */
  virtual void SetStopPosition(const uint64_t time_millisecond) { return; }
  /**
   * @brief     Suspend a player
   * @details   Player can be suspended and it will be muted. \n
   *            The current playback status will be maintained internally \n
   *            App can call suspend() when the app is switching to background
   * @pre       The player must be one of #State::kReady or #State::kPlaying or
   * #State::kPaused
   * @post      In case of success the player state will be #State::kPaused
   *            otherwise state will be not changed.
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::GetMemento() \n
   *            PlusPlayer::Restore()
   */
  virtual bool Suspend() { return false; }
  /**
   * @brief     Restore the suspended player
   * @param     [in] State : State of player after restore \n
   *              playback will be started if State::kPlaying is set.
   * @pre       The player must be suspended. (PlusPlayer::Suspend())
   * @post      The player state will be one of #State::kPlaying or
   * #State::kPaused or #State::kReady
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::Suspend()
   */
  virtual bool Restore(State state) { return false; }
  /**
   * @brief     Get current player properties
   * @param     [out] memento : PlayerMemento struct
   * @pre       The player state can be all of #State except #State::kNone
   * @return    @c True on success, otherwise @c False
   */
  virtual bool GetMemento(PlayerMemento* memento) { return false; }
  /**
   * @brief     Set the video display
   * @remarks   We are not supporting changing display.
   * @remarks   This API have to be called before calling the
   * PlusPlayer::Prepare() or PlusPlayer::PrepareAsync() to reflect the display
   * type.
   * @param     [in] type : display type
   * @param     [in] serface_id : resource id of window.
   * @param     [in] x : x param of display window
   * @param     [in] y : y param of display window
   * @param     [in] w : width of display window. This value should be greater
   * than 0.
   * @param     [in] h : height of display window. This value should be greater
   * than 0.
   * @pre       The player state can be all of #State except #State::kNone
   * @return    @c True on success, otherwise @c False
   * @see       DisplayType \n
   *            PlusPlayer::SetDisplayMode() \n
   *            PlusPlayer::SetDisplayRoi()  \n
   *            PlusPlayer::SetDisplayVisible()  \n
   */
  virtual bool SetDisplay(const DisplayType& type, const uint32_t serface_id,
                          const int x, const int y, const int w, const int h) {
    return false;
  }
  /**
   * @brief     Set the video display
   * @remarks   We are not supporting changing display.
   * @remarks   This API have to be called before calling the
   * PlusPlayer::Prepare()
   *            or PlusPlayer::PrepareAsync() to reflect the display type.
   * @param     [in] type : display type
   * @param     [in] obj : The handle to display window
   * @pre       The player state can be all of #State except #State::kNone
   * @return    @c True on success, otherwise @c False
   * @see       DisplayType
   *            PlusPlayer::SetDisplayMode()
   *            PlusPlayer::SetDisplayRoi()
   *            PlusPlayer::SetDisplayVisible()
   */
  virtual bool SetDisplay(const DisplayType& type, void* obj) { return false; }

  // plusplayer should disable mix feature temporarily.
  //#ifndef IS_AUDIO_PRODUCT
  /**
   * @brief     Set the video display
   * @remarks   We are not supporting changing display.
   * @remarks   This API have to be called before calling the
   * PlusPlayer::Prepare()
   *            or PlusPlayer::PrepareAsync() to reflect the display type.
   * @param     [in] type : display type
   * @param     [in] handle : The handle of mixer
   * @pre       The player state can be all of #State except #State::kNone
   * @post      None
   * @return    @c True on success, otherwise @c False
   * @see       DisplayType
   *            PlusPlayer::SetDisplayRoi()
   *            PlusPlayer::SetDisplayVisible()
   * @exception  None
   */
  //  virtual bool SetDisplay(const DisplayType& type, Mixer* handle) {
  //    return false;
  //  }
  //#endif

  /**
   * @brief     Set the video display mode
   * @param     [in] mode : display mode
   * @pre       The player state can be all of #State except #State::kNone
   * @return    @c True on success, otherwise @c False
   * @see       #DisplayMode
   * @see       PlusPlayer::SetDisplay()
   * @see       PlusPlayer::SetDisplayRoi()
   * @see       PlusPlayer::SetDisplayVisible()
   */
  virtual bool SetDisplayMode(const DisplayMode& mode) { return false; }
  /**
   * @brief     Set the ROI(Region Of Interest) area of display
   * @remarks   The minimum value of width and height are 1.
   * @param     [in] roi : Roi Geometry. width and height value in Geometry
   * should be greater than 0.
   * @pre       The player state can be all of #State except #State::kNone \n
   *            Before set display ROI, #DisplayMode::kDstRoi must be set with
   * PlusPlayer::SetDisplayMode().
   * @return    @c True on success, otherwise @c False
   * @see       DisplayMode  \n
   *            PlusPlayer::SetDisplay() \n
   *            PlusPlayer::SetDisplayMode() \n
   *            PlusPlayer::SetDisplayVisible()
   */
  virtual bool SetDisplayRoi(const Geometry& roi) { return false; }
  /**
   * @brief     Set the rotate angle of display
   * @remarks   The default value is 0.
   * @param     [in] rotate : Rotate angle.
   * @pre       The player state can be all of #EsState except #EsState::kNone
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::SetDisplay()
   */
  virtual bool SetDisplayRotate(const DisplayRotation& rotate) { return false; }

  /**
   * @brief     Get the rotate angle of display
   * @remarks   The default value is 0.
   * @param     [out] rotate : Stored rotate angle value.
   * @pre       The player state can be all of #EsState except #EsState::kNone
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::SetDisplayRotate()
   */
  virtual bool GetDisplayRotate(DisplayRotation* rotate) { return false; }

  /**
   * @brief     Check if video can rotate or not (video sink)
   * @remarks   The default value is False.
   * @param     [out] can_rotate : Stored rotation support value. (True/False)
   * @pre       The player state can be #State::kPlaying or kPaused.
   *            This api must be called after SetDisplay() api.
   * @return    @c True if player state is valid, otherwise @c False
   */
  virtual bool GetDisplayRotationSupport(bool& can_rotate) { return false; }

  /**
   * @brief     Check if device can rotate or not (Sero products..)
   * @remarks   The default value is False.
   * @pre       The player state can be all of #State except #State::kNone
   * @return    @c True if can rotate, otherwise @c False
   */
  virtual bool IsRotatableDevice() { return false; }

  /**
   * @brief     Set the visibility of the video display
   * @param     [in] is_visible : The visibility of the display
   *            (@c true = visible, @c false = non-visible)
   * @pre       The player state can be all of #State except #State::kNone
   * @return    @c True on success, otherwise @c False
   * @see       PlusPlayer::SetDisplay()
   */
  virtual bool SetDisplayVisible(bool is_visible) { return false; }
  /**
   * @brief     Set the ROI(Region Of Interest) area of video
   * @remarks   The minimum value of width and height are 1.
   * @param     [in] roi : Roi CropArea. width and height value in CropArea
   * should be greater than 0.
   * @pre       The player state can be all of #State except #State::kNone
   * @post      None
   * @return    @c True on success, otherwise @c False
   * @exception None
   */
  virtual bool SetVideoRoi(const CropArea& roi) { return false; }
  /**
   * @brief     Set on mute of the audio sound
   * @param     [in] is_mute : On mute of the sound
   *            (@c true = mute, @c false = non-mute)
   * @pre       The player state can be all of #State except #State::kNone
   * @return    @c True on success, otherwise @c False
   */
  virtual bool SetAudioMute(bool is_mute) { return false; }
  /**
   * @brief     Set buffer config
   * @param     [in] config : config of buffer.\n
   *             config can be \n
   *               "total_buffer_size_in_byte" \n
   *               "total_buffer_size_in_time" \n
   *               "buffer_size_in_byte_for_play" \n
   *               "buffer_size_in_sec_for_play" \n
   *               "buffer_size_in_byte_for_resume" \n
   *               "buffer_size_in_sec_for_resume" \n
   *               "buffering_timeout_in_sec_for_play"
   * @pre       The player state can be all of #State except #State::kNone
   * @return    @c True on success, otherwise @c False
   */
  virtual bool SetBufferConfig(const std::pair<std::string, int>& config) {
    return false;
  }
  /**
   * @brief     Get current state of player
   * @return    current #State of player
   */
  virtual State GetState() { return State::kNone; }
  /**
   * @brief     Get language code of the selected track.
   * @param     [in] type : track type
   * @param     [in] index : index of track
   * @pre       The player can be one of #State::kTrackSourceReady,
   * #State::kReady, #State::kPlaying or #State::kPaused
   */
  virtual std::string GetTrackLanguageCode(TrackType type, int index) {
    return {};
  }
  /**
   * @brief     Get track count of the track type.
   * @param     [in] type : track type
   * @pre       The player can be one of #State::kTrackSourceReady,
   * #State::kReady, #State::kPlaying or #State::kPaused
   * @return    Track count on success, otherwise @c -1
   */
  virtual int GetTrackCount(TrackType type) { return -1; }
  /**
   * @brief     Get track infomation of the associated media.
   * @pre       The player can be one of #State::kTrackSourceReady,
   * #State::kReady, #State::kPlaying or #State::kPaused
   * @return    std::vector of #Track infomation of stream
   */
  virtual std::vector<Track> GetTrackInfo() { return {}; }
  /**
   * @brief     Get Caption track infomation of the associated media.
   * @pre       The player can be one of #State::kTrackSourceReady,
   * #State::kReady, #State::kPlaying or #State::kPaused
   * @return    std::vector of #CaptionTrack infomation of stream
   */
  virtual std::vector<CaptionTracks> GetCaptionTrackInfo() { return {}; }
  /**
   * @brief     Get activated(selected) track infomation of the associated
   * media.
   * @remarks   In the case of Adaptive Streaming such as HLS,DASH,
   *            Track.bitrate might be different from current bitrate
   *            because it can be changed based on network bandwidth whenever.
   *            If you need the current bitrate instantly,
   *            you can get it from OnAdaptiveStreamingControlEvent().
   * @pre       The player must be one of #State::kTrackSourceReady,
   * #State::kReady, #State::kPlaying or #State::kPaused
   * @return    std::vector of activated(selected) #Track infomation of stream
   * @see       EventListener::OnAdaptiveStreamingControlEvent(kBitrateChange)
   */
  virtual std::vector<Track> GetActiveTrackInfo() { return {}; }
  /**
   * @brief     Get the duration of the stream set by url.
   * @param     [out] duration_in_milliseconds : duration in milliseconds
   * @pre       The player must be one of #State::kTrackSourceReady,
   * #State::kReady, #State::kPlaying or #State::kPaused
   * @return    @c True on success, otherwise @c False
   */
  virtual bool GetDuration(int64_t* duration_in_milliseconds) { return false; }
  /**
   * @brief     Get the current playing time of the associated media.
   * @param     [out] time_in_milliseconds : current playing time in
   * milliseconds
   * @pre       The player must be one of #State::kPlaying or #State::kPaused
   * @return    @c True on success, otherwise @c False
   *            ("time_in_milliseconds" will be 0)
   */
  virtual bool GetPlayingTime(uint64_t* time_in_milliseconds) { return false; }
  virtual bool GetTVYear(int& year) { return false; }
  /**
   * @brief     Not implemented
   */
  virtual bool SetSilentSubtitle(bool onoff) { return false; }
  /**
   * @brief     Sets playback rate
   * @param     [in]  speed : playback speed
   * @pre       The player state can be all of #State except #State::kNone &
   * #State::kIdle
   * @return	   @c True on success, otherwise @c False
   */
  virtual bool SetPlaybackRate(const double speed) { return false; }
  /**
   * @brief     Sets playback rate
   * @param     [in]  speed : playback speed
   * @param     [out] error_code : exact error code, when it's unable to set
   * given playback rate.
   * @pre       The player state can be all of #State except #State::kNone &
   * #State::kIdle
   * @return    @c True on success, otherwise @c False
   */
  virtual bool SetPlaybackRate(const double speed, ErrorType* error_code) {
    return false;
  }
  /**
   * @brief     Not implemented
   */
  virtual bool SetPlaybackRateBySeek(const double rate) { return false; }
  /**
   * @brief     Set drm property
   * @param     [in] property : drm property
   * @pre       The player state can be all of #State except #State::kNone
   */
  virtual void SetDrm(const drm::Property& property) { return; }
  /**
   * @brief     Notify drm licensing is done
   * @param     [in] type : @c which track is that drm license was acquired
   * @pre       The player must be one of #State::kTrackSourceReady or
   * #State::kReady
   */
  virtual void DrmLicenseAcquiredDone(TrackType type) { return; }
  /**
   * @brief     Set streamingengine property
   * @param     [in] type : attribute type. \n
   *             type can be \n
   *               "COOKIE" \n
   *               "USER_AGENT" \n
   *               "RESUME_TIME" \n
   *               "ADAPTIVE_INFO"
   * @param     [in] value : value of attribute type
   * @pre       The player state can be all of #State except #State::kNone
   */
  virtual void SetStreamingProperty(const std::string& type,
                                    const std::string& value) {
    return;
  }
  /**
   * @brief     Get streamingengine property
   * @param     [in] type : attribute type
   * @pre       The player state can be all of #State except #State::kNone
   */
  virtual std::string GetStreamingProperty(const std::string& type) {
    return {};
  }
  /**
   * @brief     Select track to be played
   * @param     [in] type : track type
   * @param     [in] index : index of track
   * @pre       The player state should be #State::kReady, #State::kPlaying or
   * #State::kPaused
   * @see       PlusPlyaer::GetTrackInfo()
   */
  virtual bool SelectTrack(TrackType type, int index) { return false; }
  /**
   * @brief     Register eventlistener to player
   * @param     [in] listener : listener object
   * @pre       The player state can be all of #State except #State::kNone
   * @see       EventListener
   */
  virtual void RegisterListener(EventListener* listener) { return; }
  /**
   * @brief     Register eventlistener to player
   * @param     [in] listener : listener object
   * @param     [in] userdata : listener object's userdata to be returned via
   *                            notification without any modification
   * @pre       The player state can be all of #State except #State::kNone
   * @see       EventListener
   */
  virtual void RegisterListener(EventListener* listener,
                                EventListener::UserData userdata) {
    return;
  }
  /**
   * @brief     Set external subtitle path
   * @param     [in] path : external subtitle path
   * @pre       The player state can be all of #State except #State::kNone
   */
  virtual bool SetSubtitlePath(const std::string& path) { return false; }
  /**
   * @brief     Not implemented
   */
  virtual void AdjustSubtitleSync(int offset) { return; }
  /**
   * @brief     Not implemented
   */
  virtual void SetVideoStillMode(const StillMode& type) { return; }
  /**
   * @brief     api to support stand-alone-mode.
   */
  virtual void SetWindowStandAloneMode() { return; }

  /**
   * @brief	  SetPreferredLanguage
   * @param	  [in] type : track type
   * @param	  [in] index : primary language
   * @param	  [in] index : secondary language
   * @pre 	  The player state should be #State::kIDLE
   */
  virtual bool SetPreferredLanguage(TrackType type,
                                    const std::string& primary_language,
                                    const std::string& secondary_language,
                                    const std::string& tertiary_language) {
    return false;
  }

  /**
   * @brief        Enable/Disable AI-ABR for streaming cases
   * @param        [in] status : true/false
   * @pre          The player state should be #State::kIDLE
   */
#ifdef PLUS_PLAYER_ENABLE_AI_ABR
  virtual void SetAIRequestStatus(bool status) { return; }
#endif
#ifdef PLUS_PLAYER_AI_DATA_COLLECTION
  virtual bool GetMALOGStatus() { return false; }
#endif

  /**
   * @brief	 Activates stream.
   * @param	 [in] type : track type
   * @pre 	 The player state can be all of #State except #State::kNone
   */
  virtual bool Activate(const TrackType type) { return false; }
  /**
   * @brief	 Deactivates stream.
   * @param	 [in] type : track type
   * @pre 	 The player state can be all of #State except #State::kNone
   */
  virtual bool Deactivate(const TrackType type) { return false; }
  /**
   * @brief	 Set decoder type
   * @param	 [in] type : track type
   * @param	 [in] DecoderType : hardware or software decoder type
   * @pre 	 The player state can be all of #State except #State::kNone
   */
  virtual bool SetCodecType(const TrackType type,
                            const PlayerCodecType& DecoderType) {
    return false;
  };

  /**
   * @brief     Set decoded video frame buffer type.
   * @param     [in] type : A type of decoded video frame buffer.
   * @pre       The player state must be set to #State::kIdle
   * @post      None
   * @exception  None
   */
  virtual void SetVideoFrameBufferType(const DecodedVideoFrameBufferType type) {
    return;
  }

  /**
   * @brief     Get virtual resource id
   * @param     [in] type : The resource type of virtual id.
   * @param     [out] virtual_id : Stored virtual resource id value.
   * @pre       The player state should be #State::kReady, #State::kPlaying or
   *            #State::kPaused
   * @post      None
   * @return    @c True on success, otherwise @c False ("virtual_id" will be -1)
   * @exception  None
   * @remark    This function returns virtual resource id which player is
   *            allocated from resource manager. For example, virtual scaler id
   * is required for an application to use capture API directly.
   */
  virtual bool GetVirtualRscId(const RscType type, int* virtual_id) {
    return false;
  }
  /**
   * @brief     Provided api for setting alternative video resource(sub decoder
   *            and sub scaler)
   * @param     [in] is_set : set alternative video resource
   *            (@c 0 [defualt] = set all video resources(decoder/scaler) to
   *                              main resources,
   *             @c 1 = set all video resources(decoder/scaler) to sub
   *                    resources,
   *             @c 2 = set only decoder to sub resource,
   *             @c 3 = set only scaler to sub resource)
   * @pre       The player state should be #State::kIdle
   * @return    @c True on success, otherwise @c False
   */
  virtual bool SetAlternativeVideoResource(unsigned int rsc_type) {
    return false;
  }

  /**
   * @brief     Provided api for setting advanced picture quality type
   * @param     [in] quality_type : value of picture quality type
   *            (@c kTvPlus = TV Plus)
   * @pre       The player state should be #State::kIdle
   * @return    @c True on success, otherwise @c False
   */
  virtual bool SetAdvancedPictureQualityType(
      const AdvPictureQualityType quality_type) {
    return false;
  }

  /**
  * @brief	  Below apis used for open section and pes filter callback
              from ts sgment
  * @param	  [in] pid : program id in ts pkt
  * @pre 	  The player state can be all of #State except #State::kNone
  */

  virtual void OpenSecInfoCb(int pid) { return; }
  virtual void OpenPesInfoCb(int pid) { return; }

  /**
   * @brief	  Below apis used for closing the callback set in
   * openSection/pes
   * @param	  [in] pid : program id in ts pkt
   * @pre 	  The player state should be #State::kReady, #State::kPlaying or
   * #State::kPaused
   */
  virtual void CloseSecInfo(int pid) { return; }
  virtual void ClosePesInfo(int pid) { return; }

  /**
   * @brief	  Set audio easing to avoid acoustic shock at play starts.
   * @pre 	  The player state can be all of #State except #State::kNone and
   * #State::kPlaying
   */
  virtual bool SetAntiAcousticShock() { return false; }

  /**
   * @brief  Set dashplusplayer properties via json string
   * @version 6.0
   * @pre 	  The player state required depends on the data that user try to
   * Set, if multi keys are specified, the player state should satisfy all.
   *
   *         Key name         | Required state
   *         ---------------- | -------------------------
   *         "max-bandwidth"  |  #State::kTrackSourceReady
   * @post   same as @pre
   * @exception N/A
   * @param  [in] Json formated string with { key1 : value1, key2 : value2 }
   * pairs user MUST make sure all key:value pairs are valid.
   * @note   @c data is case-sensitive. If multi keys specified, even invalid
   * key found, dashpp will still try to set the rest.
   * @return If ALL Set action excuted successfully.
   */
  virtual bool SetData(const std::string data) { return false; }

  /**
   * @brief  Get dashplusplayer properties via json string
   * @version 6.0
   * @pre 	  @see SetData()
   * @post   same as @pre
   * @exception N/A
   * @param  data Json formated string with { key1 : value1, key2 : value2 }
   * pairs, `keys` must be valid, `values` will be IGNORED as input and will be
   * filled by dashplusplayer as output.
   * @note   @c data is case-sensitive
   * @return If ALL Get action excuted successfully, if @c false user can still
   * check the data to see if any value successfully returned.
   */
  virtual bool GetData(std::string& data) { return false; }

  /**
   * @brief  Set audio volume level
   * @pre 	  The player state can be  #State::kPlaying or #State::kPaused
   * @param [in] volume  range  [0,100]
   * @return If set action excuted sucessfully
   */
  virtual bool SetVolume(int volume) { return false; }

  /**
   * @brief  Get audio volume level
   * @pre 	  The player state can be  #State::kPlaying or #State::kPaused
   * @param [out] volume  ,
   * @return If Get action excuted sucessfully
   */
  virtual bool GetVolume(int* volume) { return false; }

  /**
   * @brief  Get audio muted state
   * @pre 	  The player state can be any besides #State::kNone
   * @return If audio muted
   */
  virtual bool IsMuted() { return false; }

  virtual bool SwitchUri(int64_t ms) { return false; }

 protected:
  /**
   * @brief     Constructor
   * @param     None
   * @pre       None
   * @post      None
   * @return   None
   * @exception  None
   */
  PlusPlayer() noexcept {};
};  // class PlusPlayer

}  // namespace plusplayer

#endif  // __PLUSPLAYER_PLUSPLAYER__H__
