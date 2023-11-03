import 'dart:async';
import 'dart:convert';
import 'dart:io';

Future<Map> getTurnCredential(String host, int port) async {
  HttpClient client = HttpClient(context: SecurityContext());
  client.badCertificateCallback =
      (X509Certificate cert, String host, int port) {
    print('getTurnCredential: Allow self-signed certificate => $host:$port. ');
    return true;
  };
  final String url =
      'https://$host:$port/api/turn?service=turn&username=flutter-webrtc';
  HttpClientRequest request = await client.getUrl(Uri.parse(url));
  HttpClientResponse response = await request.close();
  final String responseBody = await response.transform(Utf8Decoder()).join();
  print('getTurnCredential:response => $responseBody.');
  Map data = JsonDecoder().convert(responseBody);
  return data;
}
