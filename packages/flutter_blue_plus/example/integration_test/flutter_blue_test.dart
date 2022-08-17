import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

// Integration test is ran alongside with nRF app (BLE simulator) which can be
// downloaded from Google Play Store/App Store.
//
// Make sure to configure the nRF app beforing running tests:
//   1. Set device name to the following name and configure advertiser to
//      broadcast "connectable" scan response data with record
//      "Complete Local Name".
const String deviceName = "TestDevice";
//   2. Configure GATT server with sample configuration which generates a
//      virtual heartrate service. The following uuids are officially reserved
//      for ble heartrate service/characteristics.
//
// Service:
const String heartrateServiceUuid = '180d';
// Characteristics:
const String heartrateMeasurementUuid = '2a37';
const String bodySensorLocationUuid = '2a38';
const String heartrateControlPointUuid = '2a39';

/// Returns a future that completes after scanning for [duration].
///
/// Returns a [ScanResult] if a device with name [deviceName] is found, otherwise
/// returns null.
Future<ScanResult?> scanRemoteDevice(
  String deviceName, [
  Duration duration = const Duration(seconds: 5),
]) async {
  Future<List<ScanResult>> scanFuture = FlutterBlue.instance
      .startScan(timeout: duration)
      .then((dynamic values) => List.castFrom<dynamic, ScanResult>(values));
  final List<ScanResult> scanResults = await scanFuture;

  for (final ScanResult scanResult in scanResults) {
    if (scanResult.device.name == deviceName) {
      return scanResult;
    }
  }
  return null;
}

/// Returns a future that completes when device is connected.
Future<void> connect(BluetoothDevice device) async {
  final Completer completer = Completer();
  await device.connect();
  final StreamSubscription subscription =
      device.state.listen((BluetoothDeviceState event) {
    if (event == BluetoothDeviceState.connected) {
      completer.complete();
    }
  });
  await completer.future;
  subscription.cancel();
}

/// Returns a future that completes when device is disconnected.
Future<void> disconnect(BluetoothDevice device) async {
  final Completer completer = Completer();
  await device.disconnect();
  final StreamSubscription subscription =
      device.state.listen((BluetoothDeviceState event) {
    if (event == BluetoothDeviceState.disconnected) {
      completer.complete();
    }
  });
  await completer.future;
  subscription.cancel();
}

// Returns a shortened version of a long 128bit uuid.
String getShortenedUuid(String uuid) {
  return uuid.substring(4, 8);
}

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('ble is available and is turned on.',
      (WidgetTester tester) async {
    final bool isAvailable = await FlutterBlue.instance.isAvailable;
    expect(isAvailable, true);

    final bool isOn = await FlutterBlue.instance.isOn;
    expect(isOn, true);
  });

  testWidgets('can scan device name: $deviceName.',
      (WidgetTester tester) async {
    Future<List<ScanResult>> scanFuture = FlutterBlue.instance
        .startScan()
        .then((dynamic values) => List.castFrom<dynamic, ScanResult>(values));

    await Future.delayed(const Duration(seconds: 1));
    final bool isScanning = await FlutterBlue.instance.isScanning.first;
    expect(isScanning, true);

    await Future.delayed(const Duration(seconds: 5));
    await FlutterBlue.instance.stopScan();
    final List<ScanResult> scanResults = await scanFuture;
    final bool deviceDiscovered = scanResults
        .any((ScanResult scanResult) => scanResult.device.name == deviceName);
    expect(deviceDiscovered, true);
  });

  testWidgets('can discover services.', (WidgetTester tester) async {
    final ScanResult? scanResult = await scanRemoteDevice(deviceName);
    expect(scanResult, isNotNull);

    final BluetoothDevice device = scanResult!.device;

    await connect(device);

    final List<BluetoothService> services = await device.discoverServices();
    BluetoothService? heartrateService;
    for (final BluetoothService service in services) {
      if (getShortenedUuid(service.uuid.toString()) == heartrateServiceUuid) {
        heartrateService = service;
      }
    }
    expect(heartrateService, isNotNull);
    final Set<String> characteristicUuids = heartrateService!.characteristics
        .map((BluetoothCharacteristic characteristic) =>
            getShortenedUuid(characteristic.uuid.toString()))
        .toSet();

    expect(
      true,
      setEquals(characteristicUuids, {
        heartrateMeasurementUuid,
        bodySensorLocationUuid,
        heartrateControlPointUuid,
      }),
    );

    await disconnect(device);
  });

  testWidgets('can setNotify/read/write characteristics.',
      (WidgetTester tester) async {
    final ScanResult? scanResult = await scanRemoteDevice(deviceName);
    final BluetoothDevice device = scanResult!.device;
    await connect(device);

    final List<BluetoothService> services = await device.discoverServices();
    late BluetoothService heartrateService;
    for (final BluetoothService service in services) {
      if (getShortenedUuid(service.uuid.toString()) == heartrateServiceUuid) {
        heartrateService = service;
      }
    }

    // Readable.
    late BluetoothCharacteristic bodySensorLocation;
    // Writable.
    late BluetoothCharacteristic heartrateControlPoint;
    for (final BluetoothCharacteristic characteristic
        in heartrateService.characteristics) {
      String shortUuid = getShortenedUuid(characteristic.uuid.toString());
      if (shortUuid == bodySensorLocationUuid) {
        bodySensorLocation = characteristic;
      } else if (shortUuid == heartrateControlPointUuid) {
        heartrateControlPoint = characteristic;
      }
    }

    expect(bodySensorLocation.lastValue, isEmpty);
    await bodySensorLocation.read();
    expect(bodySensorLocation.lastValue, isNotEmpty);

    expect(await heartrateControlPoint.write([1]), null);

    await disconnect(device);
  });
}
