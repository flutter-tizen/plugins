import 'package:flutter_driver/flutter_driver.dart';
import 'package:integration_test/common.dart' as common;
import 'package:test/test.dart';

/// This file is only used for testing of `package:integration_test` – do not
/// follow the conventions here if you are a user of `package:integration_test`.

Future<void> main() async {
  test('fails gracefully', () async {
    final FlutterDriver driver = await FlutterDriver.connect();
    final String jsonResult =
        await driver.requestData(null, timeout: const Duration(minutes: 1));
    common.Response response = common.Response.fromJson(jsonResult);
    await driver.close();
    expect(
      response.allTestsPassed,
      false,
    );
  });
}
