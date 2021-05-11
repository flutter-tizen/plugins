#ifndef VIDEO_PLAYER_OPTIONS_H_
#define VIDEO_PLAYER_OPTIONS_H_

class VideoPlayerOptions {
 public:
  VideoPlayerOptions() : mixWithOthers_(true) {}
  ~VideoPlayerOptions() = default;

  VideoPlayerOptions(const VideoPlayerOptions &other) = default;
  VideoPlayerOptions &operator=(const VideoPlayerOptions &other) = default;

  void setMixWithOthers(bool mixWithOthers) { mixWithOthers_ = mixWithOthers; }
  bool getMixWithOthers() const { return mixWithOthers_; }

 private:
  bool mixWithOthers_;
};

#endif  // VIDEO_PLAYER_OPTIONS_H_
