import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'string-extension.dart';

class Sensor extends StatelessWidget {
  final String label;
  final BluetoothCharacteristic readCharacteristic;
  final icon = {
    'temperature'   : FaIcon(FontAwesomeIcons.thermometer, size: 125),
    'accelerometer' : FaIcon(FontAwesomeIcons.running, size: 125),
    'soundlevel'    : FaIcon(FontAwesomeIcons.microphone, size: 125)
  };

  Sensor({Key key, @required this.label, @required this.readCharacteristic})
      : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text(label.toString().capitalize()),
        ),
        body: Container(
            child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
              Center(child: icon[label]),
              StreamBuilder(
                  stream: readCharacteristic.value,
                  builder: (BuildContext context, snapshot) {
                    if (snapshot.hasData && snapshot.data.length > 0) {
                      return Center(child: _buildSensorWidget(snapshot.data));
                    }
                    return CircularProgressIndicator();
                  }
              )
            ]
          )
        )
      );
  }

  Widget _buildSensorWidget(data) {
    Widget widget;

    switch (label) {
      case 'temperature':
        double tempC =
            ((((data[1] << 8) | (data[0] & 0xff)).toSigned(16) / 64.0) / 4.0) +
                25.0;

        widget = Text('$tempC\u00B0 C', style: TextStyle(fontSize: 48.0));
        break;
      case 'accelerometer':
        double accelX =
            (((data[1] << 8) | (data[0] & 0xff)).toSigned(16) / 16.0);
        double accelY =
            (((data[3] << 8) | (data[2] & 0xff)).toSigned(16) / 16.0);
        double accelZ =
            (((data[5] << 8) | (data[4] & 0xff)).toSigned(16) / 16.0);

        widget = Text('X: $accelX Y: $accelY Z: $accelZ',
            style: TextStyle(fontSize: 24.0));
        break;

      case 'soundlevel':
        int level = ((data[1] << 8) | (data[0] & 0xff)).toUnsigned(16);

        widget = Text('$level', style: TextStyle(fontSize: 48.0));
        break;
    }

    return widget;
  }
}
