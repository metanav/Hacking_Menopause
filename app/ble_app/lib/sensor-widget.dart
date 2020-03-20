import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'package:weather_icons/weather_icons.dart';
import 'string-extension.dart';

class SensorState extends State<Sensor> {
  List<int> valueBytes;

  void initState() {
    super.initState();
    valueBytes = [];
    widget.readCharacteristic.value.listen((value) {
      setState(() {
        valueBytes = value;
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    String readings;
    var icon;
    var text = Text('');

    if (valueBytes.length > 0) {
      if (widget.label == 'temperature') {
        double tempC =
            ((((valueBytes[1] << 8) | (valueBytes[0] & 0xff)).toSigned(16) / 64.0) / 4.0) + 25.0;
        print(valueBytes);
        print(tempC);
        readings = '$tempC\u00B0 C';
        icon = BoxedIcon(WeatherIcons.thermometer,
            color: Colors.pinkAccent, size: 96.0);
        text = Text('$readings', style: TextStyle(fontSize: 48.0));
      } else if (widget.label == 'accelerometer') {

        double accelX =
            (((valueBytes[1] << 8) | (valueBytes[0] & 0xff)).toSigned(16) / 16.0);
        double accelY =
            (((valueBytes[3] << 8) | (valueBytes[2] & 0xff)).toSigned(16) / 16.0);
        double accelZ =
            (((valueBytes[5] << 8) | (valueBytes[4] & 0xff)).toSigned(16) / 16.0);

        readings = 'X: $accelX Y: $accelY Z: $accelZ';
        icon = Icon(Icons.directions_run, color: Colors.pinkAccent, size: 96.0);
        text = Text('$readings', style: TextStyle(fontSize: 24.0));
      }
    }

    return Scaffold(
        appBar: AppBar(
          title: Text(widget.label.toString().capitalize()),
        ),
        body: Container(
            child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  Center(child: icon), 
                  Center(child: text.data.length > 0 ? text : CircularProgressIndicator())
                ])));
  }
}

class Sensor extends StatefulWidget {
  final String label;
  final BluetoothCharacteristic readCharacteristic;

  Sensor({Key key, @required this.label, @required this.readCharacteristic})
      : super(key: key);

  @override
  SensorState createState() => SensorState();
}
