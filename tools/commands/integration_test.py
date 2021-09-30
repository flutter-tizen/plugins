"""A subcommand that runs integration tests for multiple Tizen plugins 
under packages.

Here are some of the common use cases. Assume packages contain plugins 
a, b, c, d, and e.
- testing specific packages:
tools/run_command.py test --plugins a b c # runs a, b, c
- excluding some packages:
tools/run_command.py test --exclude a b c # runs d, e
- exclude has precedence:
tools/run_command.py test --plugins a b c --exclude b # runs a c

By default, the tool will run integration tests on all connected targets.
A target is a Tizen device (either a physical device or an emulator) that can
run Flutter applications. Generally targets are connected when device is 
connected to the host PC either with a cable or wirelessly for physical device,
and when launching targets from Tizen SDK's Emulator Manager for emulators.

Each target has three important pieces of information:
- name: the name of the target.
- platform: defines the device profile and Tizen version, usually expressed in
<device_profile>-<tizen_version> (ex: wearable-5.5, tv-6.0).
- id: the identifier assigned to a **connected** target.

Here are some of the use cases where the information can be useful:
- testing all targets that satisfies platform:
tools/run_command.py test --platform wearable-5.5
- testing target with id:
tools/run_command.py test --id some_id
"""

import os
import re
import subprocess
import sys
import time

import yaml

import commands.command_utils as command_utils

_TERM_RED = '\033[1;31m'
_TERM_GREEN = '\033[1;32m'
_TERM_YELLOW = '\033[1;33m'
_TERM_EMPTY = '\033[0m'

_LOG_PATTERN = r'\d\d:\d\d\s+([(\+\d+\s+)|(~\d+\s+)|(\-\d+\s+)]+):\s+(.*)'

_DEFAULT_PLATFORM = 'wearable-5.5'


class TestResult:
    """A class that specifies the result of a plugin integration test.

    Attributes:
        run_state: The result of the test. Can be either succeeded for failed.
        details: A list of details about the test result. (e.g. reasons for failure.)
    """

    def __init__(self, plugin_name, run_state, platform='', details=[]):
        self.plugin_name = plugin_name
        self.platform = platform
        self.run_state = run_state
        self.details = details

    @classmethod
    def success(cls, plugin_name, platform):
        return cls(plugin_name, 'succeeded', platform=platform)

    @classmethod
    def fail(cls, plugin_name, platform='', errors=[]):
        return cls(plugin_name, 'failed', platform=platform, details=errors)


def set_subparser(subparsers):
    parser = subparsers.add_parser('test', help='Run integration test')
    command_utils.set_parser_arguments(parser,
                                       plugins=True,
                                       exclude=True,
                                       run_on_changed_packages=True,
                                       base_sha=True,
                                       timeout=True,
                                       command='test')
    parser.add_argument(
        '--recipe',
        type=str,
        default='',
        help=
        '''The recipe file path. A recipe refers to a yaml file that defines a list of test targets for plugins.
Passing this file will allow the tool to test with specified targets instead of the default target, wearable-5.5.
(
plugins:
  a: [wearable-5.5, tv-6.0]
  b: [mobile-6.0]
  c: [wearable-4.0]
)''')


def _integration_test(plugin_dir, platforms, timeout):
    """Runs integration test in the example package for plugin_dir

    Currently the tools assumes that there's only one example package per plugin.

    Args:
        plugin_dir (str): The path to a single plugin directory.
        platforms (List[str]): A list of testing platforms.
        timeout (int): Time limit in seconds before cancelling the test.

    Returns:
        TestResult: The result of the plugin integration test.
    """
    plugin_name = os.path.basename(plugin_dir)

    if not platforms:
        # (TODO: HakkyuKim) Improve logic for setting default targets.
        platforms.append(_DEFAULT_PLATFORM)

    example_dir = os.path.join(plugin_dir, 'example')
    if not os.path.isdir(example_dir):
        return [
            TestResult.fail(
                plugin_name,
                errors=[
                    'Missing example directory (use --exclude if this is intentional).'
                ])
        ]

    pubspec_path = os.path.join(example_dir, 'pubspec')
    if not os.path.isfile(f'{pubspec_path}.yaml') and not os.path.isfile(
            f'{pubspec_path}.yml'):
        # TODO: Support multiple example packages.
        return [
            TestResult.fail(
                plugin_name,
                errors=['Missing pubspec file in example directory.'])
        ]

    integration_test_dir = os.path.join(example_dir, 'integration_test')
    if not os.path.isdir(integration_test_dir) or not os.listdir(
            integration_test_dir):
        return [
            TestResult.fail(
                plugin_name,
                errors=[
                    'Missing integration tests (use --exclude if this is intentional).'
                ])
        ]

    try:
        errors = []
        completed_process = subprocess.run('flutter-tizen pub get',
                                           shell=True,
                                           cwd=example_dir,
                                           stderr=subprocess.PIPE,
                                           stdout=subprocess.PIPE)
        if completed_process.returncode != 0:
            if not completed_process.stderr:
                errors.append('pub get failed. Make sure the pubspec file \
                        in your project is valid.')
            else:
                errors.append(completed_process.stderr)
            return [TestResult.fail(plugin_name, errors=errors)]

        test_results = []
        id_per_platform = _get_target_table()

        for platform in platforms:
            if platform not in id_per_platform:
                test_results.append(
                    TestResult.fail(
                        plugin_name, platform,
                        [f'Test runner cannot find target {platform}.']))
                continue

            is_timed_out = False
            process = subprocess.Popen(
                f'flutter-tizen -d {id_per_platform[platform]} test integration_test',
                shell=True,
                cwd=example_dir,
                universal_newlines=True,
                stderr=subprocess.PIPE,
                stdout=subprocess.PIPE)
            last_line = ''
            start = time.time()
            for line in process.stdout:
                match = re.search(_LOG_PATTERN, line)
                last_match = re.search(_LOG_PATTERN, last_line)
                if match and last_match and last_match.group(2) == match.group(
                        2):
                    sys.stdout.write(f'\r{line.strip()}')
                else:
                    sys.stdout.write(f'\n{line.strip()}')
                sys.stdout.flush()
                last_line = line
                if time.time() - start > timeout:
                    process.kill()
                    is_timed_out = True
                    break
            sys.stdout.write('\n')
            sys.stdout.flush()
            process.wait()

            if is_timed_out:
                errors.append(
                    """Timeout expired. The test may need more time to finish.
        If you expect the test to finish before timeout, check if the tests 
        require device screen to be awake or if they require manually 
        clicking the UI button for permissions.""")
                test_results.append(
                    TestResult.fail(plugin_name, platform, errors=errors))
                continue
            if last_line.strip() == 'No tests ran.':
                # This message occurs when the integration test file exists,
                # but no actual test code is written in it.
                test_results.append(
                    TestResult.fail(plugin_name, platform, [
                        'Missing integration tests (use --exclude if this is intentional).'
                    ]))
                continue
            elif last_line.strip().startswith('No devices found'):
                test_results.append(
                    TestResult.fail(
                        plugin_name, platform,
                        ['The runner cannot find any devices to run tests.']))
                continue

            match = re.search(_LOG_PATTERN, last_line.strip())
            if not match:
                test_results.append(
                    TestResult.fail(
                        plugin_name, platform,
                        ['Log message is not formatted correctly.']))
                continue

            # In some cases, the command returns 0 for failed cases, so we check again
            # with the last log message.
            exit_code = process.returncode
            if match.group(2) == 'All tests passed!':
                exit_code = 0
            elif match.group(2) == 'Some tests failed.':
                errors.append(
                    'flutter-tizen test integration_test failed, see the output above for details.'
                )
                exit_code = 1

            if exit_code == 0:
                test_results.append(TestResult.success(plugin_name, platform))
            else:
                test_results.append(
                    TestResult.fail(plugin_name, platform, errors=errors))
    finally:
        subprocess.run('flutter-tizen clean', shell=True, cwd=example_dir)

    return test_results


def _get_target_table():

    def _parse_target_info(capability_info: str):
        capability_info.rstrip()
        device_profile = ''
        tizen_version = ''
        for line in capability_info.split('\n'):
            tokens = line.split(':')
            if (tokens[0] == 'profile_name'):
                device_profile = tokens[1]
            elif (tokens[0] == 'platform_version'):
                tizen_version = tokens[1]
        return device_profile, tizen_version

    completed_process = subprocess.run('sdb devices',
                                       shell=True,
                                       cwd='.',
                                       universal_newlines=True,
                                       stderr=subprocess.PIPE,
                                       stdout=subprocess.PIPE)

    id_per_platform = {}
    if completed_process.returncode == 0:
        lines = completed_process.stdout.rstrip().split('\n')
        for line in lines[1:]:
            id = line.split(' ')[0]
            completed_process = subprocess.run(f'sdb -s {id} capability',
                                               shell=True,
                                               cwd='.',
                                               universal_newlines=True,
                                               stderr=subprocess.PIPE,
                                               stdout=subprocess.PIPE)
            device_profile, tizen_version = _parse_target_info(
                completed_process.stdout)
            if not device_profile or not tizen_version:
                print(
                    f'''Cannot extract device_profile or tizen_version information from device {id}.
device_profile: {device_profile}
tizen_version: {tizen_version}''')
            platform = f'{device_profile}-{tizen_version}'
            if platform in id_per_platform:
                print(
                    f'Multiple targets of {platform} found. Replacing {id_per_platform[platform]} to {id}...'
                )
            id_per_platform[platform] = id
    return id_per_platform


def run_integration_test(args):
    platforms_per_plugin = {}
    if args.recipe:
        if not os.path.isfile(args.recipe):
            print(f'The recipe file {args.recipe} does not exist.')
            exit(1)
        with open(args.recipe) as f:
            try:
                platforms_per_plugin = yaml.load(
                    f.read(), Loader=yaml.FullLoader)['plugins']
            except yaml.parser.ParserError:
                print(
                    f'The recipe file {args.recipe} is not a valid yaml file.')
                exit(1)

    packages_dir = command_utils.get_package_dir()
    testing_plugins, excluded_plugins = command_utils.get_target_plugins(
        packages_dir,
        plugins=args.plugins,
        exclude=args.exclude,
        run_on_changed_packages=args.run_on_changed_packages,
        base_sha=args.base_sha)

    test_num = 0
    total_plugin_num = len(testing_plugins)
    results = []
    for testing_plugin in testing_plugins:
        test_num += 1
        print(
            f'============= Testing for {testing_plugin} ({test_num}/{total_plugin_num}) ============='
        )
        platforms = []
        if testing_plugin in platforms_per_plugin:
            platforms = platforms_per_plugin[testing_plugin]

        results.extend(
            _integration_test(os.path.join(packages_dir, testing_plugin),
                              platforms, args.timeout))

    print(f'============= TEST RESULT =============')
    failed_plugins = []
    for result in results:
        color = _TERM_GREEN
        if result.run_state == 'failed':
            color = _TERM_RED

        print(
            f'{color}{result.run_state.upper()}: {result.plugin_name} {result.platform}{_TERM_EMPTY}'
        )
        if result.run_state != 'succeeded':
            for detail in result.details:
                print(f'{detail}')
        if result.run_state == 'failed':
            failed_plugins.append(testing_plugin)

    exit_code = 0
    if len(failed_plugins) > 0:
        print(f'{_TERM_RED}Some tests failed.{_TERM_EMPTY}')
        exit_code = 1
    elif total_plugin_num == 0:
        print('No tests ran.')
    else:
        print(f'{_TERM_GREEN}All tests passed!{_TERM_EMPTY}')

    for excluded_plugin in excluded_plugins:
        print(f'{_TERM_YELLOW}EXCLUDED: {excluded_plugin}{_TERM_EMPTY}')
    exit(exit_code)
