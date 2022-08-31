import 'parcel.dart';

abstract class Parcelable {
  void serialize(Parcel parcel);
  void deserialize(Parcel parcel);
}
