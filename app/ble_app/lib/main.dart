import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(title: 'BLE Scanner', home: Devices());
  }
}

class DevicesState extends State<Devices> {
  FlutterBlue flutterBlue = FlutterBlue.instance;
  List<ScanResult> _discovered = <ScanResult>[];
  final TextStyle _biggerFont = TextStyle(fontSize: 18.0);
  final GlobalKey<ScaffoldState> _scaffoldKey = new GlobalKey<ScaffoldState>();

  Widget _buildDeviceList() {
    return ListView.builder(
        itemCount: _discovered.length,
        padding: const EdgeInsets.all(16.0),
        itemBuilder: (context, i) {
          if (i.isOdd) return Divider();
          return _buildRow(_discovered[i]);
        });
  }

  Widget _buildRow(ScanResult result) {
    return ListTile(
      title: Text(
        (result.device.name.length > 0) ? result.device.name : "<NO NAME>",
        style: _biggerFont,
      ),
      trailing: Text(result.rssi.toString(), style: _biggerFont),
      onTap: () => _pushDevices(result.device),
    );
  }

  Future<void> _pushDevices(device) async {
    try {
      await device.connect();
      Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => Services(device: device),
        ),
      );
    } catch (err) {
      final snackBar = SnackBar(content: Text(err.toString()));
      _scaffoldKey.currentState.showSnackBar(
        SnackBar(
          content: new Text('Error: ${err}'),
          duration: new Duration(seconds: 10),
        )
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      key: _scaffoldKey,
      appBar: AppBar(
        title: Text('BLE Scanner'),
        actions: <Widget>[
          IconButton(icon: Icon(Icons.scanner), onPressed: _scan),
        ],
      ),
      body: _buildDeviceList(),
    );
  }

  void _scan() {
    flutterBlue.startScan(timeout: Duration(seconds: 10));
    flutterBlue.scanResults.listen((result) {
      if (result.length > 0) {
        setState(() {
          _discovered = result;
        });
      }
    });
    flutterBlue.stopScan();
  }
}

class Devices extends StatefulWidget {
  @override
  DevicesState createState() => DevicesState();
}

class Services extends StatelessWidget {
  final BluetoothDevice device;

  Services({Key key, @required this.device}) : super(key: key) {
    this.device.connect();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(device.name.toString()),
      ),
      body: Padding(
        padding: EdgeInsets.all(16.0),
        child: Text(this.device.name.toString()),
      ),
    );
  }
}
