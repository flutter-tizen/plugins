<br>
<p align="center">
<img alt="FlutterBlue" src="https://github.com/pauldemarco/flutter_blue/blob/master/site/flutterblue.png?raw=true" />
</p>
<br><br>

## Introduction

FlutterBlue is a bluetooth plugin for [Flutter](https://flutter.dev), a new app SDK to help developers build modern multi-platform apps.

## Alpha version

This library is actively developed alongside production apps, and the API will evolve as we continue our way to version 1.0.

**Please be fully prepared to deal with breaking changes.**
**This package must be tested on a real device.**

Having trouble adapting to the latest API?   I'd love to hear your use-case, please contact me.

## Cross-Platform Bluetooth LE
FlutterBlue aims to offer the most from both platforms (iOS, Android and Tizen).

Using the FlutterBlue instance, you can scan for and connect to nearby devices ([BluetoothDevice](#bluetoothdevice-api)).
Once connected to a device, the BluetoothDevice object can discover services ([BluetoothService](lib/src/bluetooth_service.dart)), characteristics ([BluetoothCharacteristic](lib/src/bluetooth_characteristic.dart)), and descriptors ([BluetoothDescriptor](lib/src/bluetooth_descriptor.dart)).
The BluetoothDevice object is then used to directly interact with characteristics and descriptors.

## Usage
### Obtain an instance
```dart
FlutterBlue flutterBlue = FlutterBlue.instance;
```

### Scan for devices
```dart
// Start scanning
flutterBlue.startScan(timeout: Duration(seconds: 4));

// Listen to scan results
var subscription = flutterBlue.scanResults.listen((results) {
    // do something with scan results
    for (ScanResult r in results) {
        print('${r.device.name} found! rssi: ${r.rssi}');
    }
});

// Stop scanning
flutterBlue.stopScan();
```

### Connect to a device
```dart
// Connect to the device
await device.connect();

// Disconnect from device
device.disconnect();
```

### Discover services
```dart
List<BluetoothService> services = await device.discoverServices();
services.forEach((service) {
    // do something with service
});
```

### Read and write characteristics
```dart
// Reads all characteristics
var characteristics = service.characteristics;
for(BluetoothCharacteristic c in characteristics) {
    List<int> value = await c.read();
    print(value);
}

// Writes to a characteristic
await c.write([0x12, 0x34])
```

### Read and write descriptors
```dart
// Reads all descriptors
var descriptors = characteristic.descriptors;
for(BluetoothDescriptor d in descriptors) {
    List<int> value = await d.read();
    print(value);
}

// Writes to a descriptor
await d.write([0x12, 0x34])
```

### Set notifications and listen to changes
```dart
await characteristic.setNotifyValue(true);
characteristic.value.listen((value) {
    // do something with new value
});
```

### Read the MTU and request larger size
```dart
final mtu = await device.mtu.first;
await device.requestMtu(123);
```

## Getting Started
### **Tizen**
In the **tizen-manifest.xml** let's add
```xml
<privileges>
    <privilege>http://tizen.org/privilege/bluetooth</privilege>
</privileges>
```



## Reference
### FlutterBlue API
|                  |         Tizen          |             Description            |
| :--------------- | :------------------: |  :-------------------------------- |
| scan             |  :white_check_mark:  | Starts a scan for Bluetooth Low Energy devices. |
| state            |  :white_check_mark:  | Stream of state changes for the Bluetooth Adapter. |
| isAvailable      |  :white_check_mark:  | Checks whether the device supports Bluetooth. |
| isOn             |  :white_check_mark:  | Checks if Bluetooth functionality is turned on. |

### BluetoothDevice API
|                             |         Tizen        |             Description            |
| :-------------------------- | :------------------: |  :-------------------------------- |
| connect                     |  :white_check_mark:  | Establishes a connection to the device. |
| disconnect                  |  :white_check_mark:  | Cancels an active or pending connection to the device. |
| discoverServices            |  :white_check_mark:  | Discovers services offered by the remote device as well as their characteristics and descriptors. |
| services                    |  :white_check_mark:  | Gets a list of services. Requires that discoverServices() has completed. |
| state                       |  :white_check_mark:  | Stream of state changes for the Bluetooth Device. |
| mtu                         |  :white_check_mark:  | Stream of mtu size changes. |
| requestMtu                  |  :white_check_mark:  | Request to change the MTU for the device. |

### BluetoothCharacteristic API
|                             |         Tizen        |             Description            |
| :-------------------------- | :------------------: |  :-------------------------------- |
| read                        |  :white_check_mark:  | Retrieves the value of the characteristic.  |
| write                       |  :white_check_mark:  | Writes the value of the characteristic. |
| setNotifyValue              |  :white_check_mark:  | Sets notifications or indications on the characteristic. |
| value                       |  :white_check_mark:  | Stream of characteristic's value when changed. |

### BluetoothDescriptor API
|                             |         Tizen        |             Description            |
| :-------------------------- | :------------------: |  :-------------------------------- |
| read                        |  :white_check_mark:  | Retrieves the value of the descriptor.  |
| write                       |  :white_check_mark:  | Writes the value of the descriptor. |

## Troubleshooting
### When I scan using a service UUID filter, it doesn't find any devices.
Make sure the device is advertising which service UUID's it supports.  This is found in the advertisement
packet as **UUID 16 bit complete list** or **UUID 128 bit complete list**.

Plugin is supported on Tizen 6.5 iot-headed armv7l, aarch64 (RPI4, RB5)