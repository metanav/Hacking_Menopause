import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'sensor-widget.dart';
import 'string-extension.dart';

class ServicesState extends State<Services> {
  Widget _buildServiceList() {
    final services = widget.services.where((x) => 
      widget.sensorServices.keys.contains(x.uuid.toString())
    ).toList();

    return ListView.builder(
        itemCount: services.length,
        padding: const EdgeInsets.all(16.0),
        itemBuilder: (context, i) {
            return _buildRow(services[i]);
        });
  }

  Widget _buildRow(BluetoothService service) {
    return ListTile(
      title: Text(
        widget.sensorServices[service.uuid.toString()].capitalize(),
        style: TextStyle(fontSize: 18.0),
      ),
      //trailing: Text(result.rssi.toString(), style: _biggerFont),
      onTap: () => _pushSensor(service.characteristics),
    );
  }

  Future<void> _pushSensor(characteristics) async {
    BluetoothCharacteristic readCharacteristic;
    String label;

    for (BluetoothCharacteristic characteristic in characteristics) {
      var sensor = widget.sensorServices[characteristic.serviceUuid.toString()];
      print(sensor);

      if (characteristic.uuid.toString() ==
          widget.sensorCharacteristics[sensor]['config']) {
        await characteristic.write([0x01]);
      }

      if (characteristic.uuid.toString() ==
          widget.sensorCharacteristics[sensor]['read']) {
        readCharacteristic = characteristic;
        label = widget.sensorServices[readCharacteristic.serviceUuid.toString()];
        await readCharacteristic.setNotifyValue(true);
      }
      // List<int> value = await characteristic.read();

    }
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => Sensor(label: label, readCharacteristic: readCharacteristic),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text('Services'),
        ),
        body: _buildServiceList());
  }
}

class Services extends StatefulWidget {
  final List<BluetoothService> services;
  final sensorServices = {
    '00003000-0000-1000-8000-00805f9b34fb': 'accelerometer',
    '00003010-0000-1000-8000-00805f9b34fb': 'temperature'
  };

  final sensorCharacteristics = {
    'temperature': {
      'read'   : '00003011-0000-1000-8000-00805f9b34fb',
      'config' : '00003012-0000-1000-8000-00805f9b34fb'
    },
    'accelerometer': {
      'read'   : '00003001-0000-1000-8000-00805f9b34fb',
      'config' : '00003003-0000-1000-8000-00805f9b34fb'
    },
  };

  Services({Key key, @required this.services}) : super(key: key);

  @override
  ServicesState createState() => ServicesState();
}

