/// The disposable interface for consistency.
abstract class Disposable {
  /// The flag whether the object is disposed.
  bool isDisposed = false;

  /// Dispose the object.
  void dispose() {}
}
