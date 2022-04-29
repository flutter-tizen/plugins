#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_OPTIONS_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_OPTIONS_H_

class VideoPlayerOptions {
 public:
  VideoPlayerOptions() {}
  ~VideoPlayerOptions() = default;

  VideoPlayerOptions(const VideoPlayerOptions &other) = default;
  VideoPlayerOptions &operator=(const VideoPlayerOptions &other) = default;

  void setMixWithOthers(bool mix_with_others) {
    mix_with_others_ = mix_with_others;
  }
  bool getMixWithOthers() const { return mix_with_others_; }

 private:
  bool mix_with_others_ = true;
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_OPTIONS_H_
