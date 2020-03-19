import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'package:weather_icons/weather_icons.dart';
import 'string-extension.dart';

class SensorState extends State<Sensor> {
  double readings;

  void initState() {
    super.initState();
    readings = 0;
    widget.readCharacteristic.value.listen((value) {
      setState(() {
        readings =  (((value[1] << 8 ) | (value[0] & 0xff)) / 64.0 ) / 4.0 + 25.0;
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text(widget.label.toString().capitalize()),
        ),
        body: Container(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              Center(
                child: BoxedIcon(
                  WeatherIcons.thermometer,
                  color: Colors.pinkAccent,
                  size: 96.0,
                  )
              ),
              Center(
                child: Text('$readings\u00B0 C', style: TextStyle(fontSize: 48.0),)
              )
            ]
          )
        )
    );
  }
}

class Sensor extends StatefulWidget {
  final String label;
  final BluetoothCharacteristic readCharacteristic;

  Sensor({Key key,@required this.label,  @required this.readCharacteristic}) : super(key: key);

  @override
  SensorState createState() => SensorState();
}
