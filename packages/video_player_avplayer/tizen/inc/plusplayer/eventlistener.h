/**
 * @file           eventlistener.h
 * @interfacetype  module
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        1.0
 * @SDK_Support    N
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

#ifndef __PLUSPLAYER_EVENTLISTENER__H__
#define __PLUSPLAYER_EVENTLISTENER__H__

#include "plusplayer/drm.h"
#include "plusplayer/track.h"
#include "plusplayer/types/error.h"
#include "plusplayer/types/streaming_message.h"
#ifndef PLUS_PLAYER_AI_DATA_COLLECTION
#define PLUS_PLAYER_AI_DATA_COLLECTION
#endif


namespace plusplayer {

/**
 * @brief   Interface of player EventListener
 * @details You must implement concrete class
 *          and register it to get player events.
 * @see     PlusPlayer::RegisterListener()
 */
class EventListener {
 public:
  EventListener() noexcept {}
  using UserData = void*;
  virtual ~EventListener() {}
  /**
   * @brief     It will be invoked when error has happened
   * @param     [in] error_code : #ErrorType
   * @see       #ErrorType
   * @see       OnErrorMsg() if a detailed error message is required
   */
  virtual void OnError(const ErrorType& error_code, UserData userdata) {}

  /**
   * @brief     It will be invoked when error has happened
   * @param     [in] error_code : #ErrorType
   * @param     [in] error_msg : detailed error message including info related
   *                 to codec,demuxer,network status, etc.
   * @see       #ErrorType
   * @see       OnError() if only simple error code is required
   */
  virtual void OnErrorMsg(const ErrorType& error_code, const char* error_msg,
                          UserData userdata) {}

  /**
   * @brief     It will be invoked during buffering
   * @param     [in] percent : current buffer status in percent
   */
  virtual void OnBufferStatus(const int percent, UserData userdata) {}
  /**
   * @brief     It will be invoked when H/W resource is conflicted
   * @param     [in] userdata of event
   * @pre       None
   * @post      None
   * @exception None
   */
  virtual void OnResourceConflicted(UserData userdata) {}
  /**
   * @brief     It will be invoked when player has reached the end of the stream
   */
  virtual void OnEos(UserData userdata) {}

  /**
   * @brief     It will be invoked when player gets closed caption data from
   * decoder
   * @param     [in] data : closed caption data
   * @param     [in] size : size of closed caption data
   */
  virtual void OnClosedCaptionData(std::unique_ptr<char[]> data, const int size,
                                   UserData userdata) {}
  /**
   * @brief     It will be invoked when streamingengine invokes adaptive
   * streaming control event
   * @param     [in] type : #StreamingMessageType
   * @param     [in] MessageParam : #MessageParam
   */
  virtual void OnAdaptiveStreamingControlEvent(const StreamingMessageType& type,
                                               const MessageParam& msg,
                                               UserData userdata) {}
  /**
* @brief     It will be invoked when queue in hls event triggered for cue
* advetisment start
* @param     [in] msgType : #event name
* @param     [in] MessageParam : #MessageParam
*/
  virtual void OnCueEvent(const char *CueData,
                                               UserData userdata) {}
  /**
* @brief     It will be invoked when InteractiveAd event triggered 
* @param     [in] msgType : #event name
* @param     [in] MessageParam : #MessageParam
*/
  virtual void OnInteractiveAd(const char *InteractiveAdData,
                                               UserData userdata) {}
   /**
* @brief     It will be invoked when InteractiveAd start 
* @param     [in] msgType : #event name
* @param     [in] MessageParam : #MessageParam
*/
  virtual void OnInteractiveAdStart(const char *InteractiveAdID,
                                               UserData userdata) {}

     /**
* @brief     It will be invoked when InteractiveAd stop reaches
* @param     [in] msgType : #event name
* @param     [in] MessageParam : #MessageParam
*/
  virtual void OnInteractiveAdStop(const char *InteractiveAdID,
                                               UserData userdata) {}

  virtual void OnDateRangeEvent(const char* DateRangeData, UserData userdata) {}
  virtual void OnStopReachEvent(bool StopReach, UserData userdata) {}

  virtual void OnCueOutContEvent(const char* CueOutContData,
                                 UserData userdata) {}     
								 
    virtual void OnSwitchDoneEvent(UserData userdata) {}                         
  /**
   * @brief     It will be invoked when dash scte:35 cue event is parsed
   * @param     [in] Ad : # Ad information c_str ends with '\0'
   * @pre       This callback will not be invoked before player prepare 
   * @post      None
   * @exception None
   */
  virtual void OnADEventFromDash(const char* ADData, UserData userdata) {}

  /**
   * @brief     It will be invoked when HbbTV preselection case by using dashplusplayer with appid "org.tizen.hbbtv"
   * @param     [in] ps_data : string stored preselection info followed json format
   * @pre       This callback will not be invoked before player prepare 
   * @post      None
   * @exception None
   */
  virtual void OnAudioPreselectionInfoFromDash(const char * PsData, UserData userdata) {}

  /**
   * @brief     It will be invoked when player is prepared to be started
   * @details   This will be invoked when user calls PlusPlayer::PrepareAsync()
   * @param     [in] ret : statue of prepare (@c true = success, @c false =
   * fail)
   * @see       PlusPlayer::PrepareAsync()
   */
  virtual void OnPrepareDone(bool ret, UserData userdata) {}
  /**
   * @brief     It will be invoked when the seek operation is completed
   * @remarks   OnSeekDone() will be called once seek operation is finished
   * @see       PlusPlayer::Seek()
   */
  virtual void OnSeekDone(UserData userdata) {}
  /**
   * @brief     It will be invoked when changing source
   * @details   This will be invoked when user calls PlusPlayer::ChangeSource()
   * @remarks   You must call Start()(or Pause()) for Playing(or Pause) after
   * ChangeSource()
   * @param     [in] ret : statue of change source (@c true = success, @c false
   * = fail)
   * @post      The player state will be #State::kReady ? #State::kPlaying
   * @see       PlusPlayer::ChangeSource()
   */
  virtual void OnChangeSourceDone(bool ret, UserData userdata) {}
  /**
   * @brief     It will be invoked when the subtitle is updated.
   * @details   All of internal/external subtitle except closed caption.
   * @param     [in] data : subtitle data
   * @param     [in] size : size of subtitle data
   * @param     [in] type : type of subtitle data
   * @param     [in] duration : duration of subtitle
   * @param     [in] attr_list : attribute list of subtitle
   * @see       #SubtitleType
   */
  virtual void OnSubtitleData(std::unique_ptr<char[]> data, const int size,
                              const SubtitleType& type, const uint64_t duration,
                              SubtitleAttrListPtr attr_list,
                              UserData userdata) {}
  /**
  * @brief      It will be invoked when player state was changed to playing
  * @param      [in] userdata : userdata of event
  * @pre        None
  * @post       The player state will be #State::kPlaying
  * @return     None
  * @exception  None
  */
  virtual void OnStateChangedToPlaying(UserData userdata) {}
#ifdef DRM_MAPI_AARCH_64  
  virtual void OnDrmInitData(unsigned long* drmhandle, unsigned int len,
#else
  virtual void OnDrmInitData(int* drmhandle, unsigned int len,
#endif		  
                             unsigned char* psshdata, TrackType type,
                             UserData userdata) {}
/**
* @brief      It will be invoked to post AI data collection
* @param      [in] userdata : userdata of event
* @pre        None
* @post       The player state will be #State::kPlaying
* @return     None
* @exception  None
*/
#ifdef PLUS_PLAYER_AI_DATA_COLLECTION
  virtual void OnAIDataCollection(std::unique_ptr<char[]> ai_data,
                                  UserData userdata) {}
#endif

#ifdef PLUS_PLAYER_ENABLE_ERROR_FRAMEWORK
  virtual void OnErrorFW(std::unique_ptr<char[]> error_data,
                         UserData userdata) {}
#endif

  /**
  * @brief      It will be invoked when demux found drm caps
  * @param      [in] userdata : userdata of event
  * @return     None
  * @exception  None
  */
  virtual void OnDrmType(plusplayer::drm::Type drmtype, UserData userdata) {}

  /**
  * @brief      It will be invoked when pes data received from demux
  * @param	   [in] pid : pkt indetifier
  * @param	   [in] data : pes data
  * @param	   [in] size : size of pes data
  * @param	   [in] userdata : userdata of event
  * @return     None
  * @exception  None
  */
  virtual void OnPesData(int pid, std::unique_ptr<char[]> data, 
                             const int size,UserData userdata) {}
  
  virtual void  OnPSSHData(const StreamingMessageType& type,
                                  const MessageParam& msg, UserData userdata){}
  
  /**
  * @brief      It will be invoked when section data received from demux
  * @param	   [in] pid : pkt indetifier
  * @param	   [in] data : section data
  * @param	   [in] size : size of section data
  * @param	   [in] userdata : userdata of event
  * @return     None
  * @exception  None
  */

  virtual void OnSectionData(int pid, std::unique_ptr<char[]> data,
                             const int size,UserData userdata) {}
#ifdef PLUS_PLAYER_ENABLE_ERROR_FRAMEWORK
  virtual void OnErrorFW(std::unique_ptr<char[]> error_data, UserData userdata){}
#endif 

};

}  // namespace plusplayer

#endif  // __PLUSPLAYER_EVENTLISTENER__H__
