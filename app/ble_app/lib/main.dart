import 'package:flutter/material.dart';
import 'devices.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(title: 'BLE Scanner', home: Devices());
  }
}
