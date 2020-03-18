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
          //if (i.isOdd) return Divider();
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
      List<BluetoothService> services = await device.discoverServices();

      Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => Services(services: services),
        ),
      );
    } catch (err) {
      final snackBar = SnackBar(content: Text(err.toString()));
      _scaffoldKey.currentState.showSnackBar(SnackBar(
        content: new Text('Error: ${err}'),
        duration: new Duration(seconds: 10),
      ));
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

class ServicesState extends State<Services> {
  Widget _buildServiceList() {
    return ListView.builder(
        itemCount: widget.services.length,
        padding: const EdgeInsets.all(16.0),
        itemBuilder: (context, i) {
          if (widget.allowedServices.keys
              .contains(widget.services[i].uuid.toString())) {
            return _buildRow(widget.services[i]);
          }
        });
  }

  Widget _buildRow(BluetoothService service) {
    return ListTile(
      title: Text(
        widget.allowedServices[service.uuid.toString()],
        style: TextStyle(fontSize: 18.0),
      ),
      //trailing: Text(result.rssi.toString(), style: _biggerFont),
      onTap: () => _pushSensor(service.characteristics),
    );
  }

  Future<void> _pushSensor(characteristics) async {
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => Sensor(characteristics: characteristics),
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
  final allowedServices = {
    '00003000-0000-1000-8000-00805f9b34fb': 'Accelerometer',
    '00003010-0000-1000-8000-00805f9b34fb': 'Temperature'
  };

  Services({Key key, @required this.services}) : super(key: key);

  @override
  ServicesState createState() => ServicesState();
}

class SensorState extends State<Sensor> {
  Widget _buildSensorUI() async {
    for (BluetoothCharacteristic characteristic in widget.characteristics) {
      if (characteristic.uuid.toString() == '00003012-0000-1000-8000-00805f9b34fb') {
        List<int> value = await characteristic.read();
        print(value);
      }
      // Writes to a characteristic
      //await c.write([0x12, 0x34])
    }
    return Text(widget.characteristics.length.toString());
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text(''),
        ),
        body: _buildSensorUI());
  }
}

class Sensor extends StatefulWidget {
  final List<BluetoothCharacteristic> characteristics;

  Sensor({Key key, @required this.characteristics}) : super(key: key);

  @override
  SensorState createState() => SensorState();
}
