import 'parcel.dart';

/// The parcelable class that can be encode & decode raw data.
abstract class Parcelable {
  /// Serialize self to raw data. And write it to parcel.
  void serialize(Parcel parcel);

  /// Desrialize data from parcel.
  void deserialize(Parcel parcel);
}
