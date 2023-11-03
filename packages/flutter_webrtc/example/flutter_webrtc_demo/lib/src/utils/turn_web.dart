import 'dart:convert';
import 'package:http/http.dart';

Future<Map> getTurnCredential(String host, int port) async {
  final String url =
      'https://$host:$port/api/turn?service=turn&username=flutter-webrtc';
  final Response res = await get(Uri.parse(url));
  if (res.statusCode == 200) {
    var data = json.decode(res.body);
    print('getTurnCredential:response => $data.');
    return data;
  }
  return {};
}
