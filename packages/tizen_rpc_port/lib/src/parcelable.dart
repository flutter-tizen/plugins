import 'parcel.dart';

/// The parcelable class that can be serialize & deserialize object data.
abstract class Parcelable {
  /// Serializes the object data to the parcel.
  void serialize(Parcel parcel);

  /// Desrializes the object data from the parcel.
  void deserialize(Parcel parcel);
}
