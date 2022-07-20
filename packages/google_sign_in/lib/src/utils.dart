import 'dart:convert' as convert;

Map<String, dynamic> decodeJWT(String token) {
  final List<String> splitTokens = token.split('.');
  if (splitTokens.length != 3) {
    throw const FormatException('Invalid token.');
  }
  final String normalizedPayload = convert.base64.normalize(splitTokens[1]);
  final String payloadString =
      convert.utf8.decode(convert.base64.decode(normalizedPayload));
  return convert.jsonDecode(payloadString) as Map<String, dynamic>;
}

void checkFormat<T>(List<String> names, Map<String, dynamic> parameters) {
  for (final String name in names) {
    final dynamic value = parameters[name];
    if (value != null && value is! T) {
      throw FormatException('parameter "$name" is not a $T, is "$value".');
    }
  }
}
