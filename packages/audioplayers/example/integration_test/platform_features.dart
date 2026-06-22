/// Specify supported features for a platform.
class PlatformFeatures {
  final bool hasUrlSource;
  final bool hasDataUriSource;
  final bool hasAssetSource;
  final bool hasBytesSource;

  final bool hasPlaylistSourceType;

  final bool hasLowLatency;
  final bool hasReleaseModeRelease;
  final bool hasReleaseModeLoop;
  final bool hasVolume;
  final bool hasBalance;
  final bool hasSeek;
  final bool hasMp3Duration;

  final bool hasPlaybackRate;
  final bool hasForceSpeaker;
  final bool hasDuckAudio;
  final bool hasRespectSilence;
  final bool hasStayAwake;
  final bool hasRecordingActive;
  final bool hasPlayingRoute;

  final bool hasDurationEvent;
  final bool hasPlayerStateEvent;
  final bool hasErrorEvent;

  const PlatformFeatures({
    this.hasUrlSource = true,
    this.hasDataUriSource = true,
    this.hasAssetSource = true,
    this.hasBytesSource = true,
    this.hasPlaylistSourceType = true,
    this.hasLowLatency = true,
    this.hasReleaseModeRelease = true,
    this.hasReleaseModeLoop = true,
    this.hasMp3Duration = true,
    this.hasVolume = true,
    this.hasBalance = true,
    this.hasSeek = true,
    this.hasPlaybackRate = true,
    this.hasForceSpeaker = true,
    this.hasDuckAudio = true,
    this.hasRespectSilence = true,
    this.hasStayAwake = true,
    this.hasRecordingActive = true,
    this.hasPlayingRoute = true,
    this.hasDurationEvent = true,
    this.hasPlayerStateEvent = true,
    this.hasErrorEvent = true,
  });

  factory PlatformFeatures.instance() => const PlatformFeatures();
}
