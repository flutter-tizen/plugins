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
from collections import defaultdict

import yaml

import commands.command_utils as command_utils

_TERM_RED = '\033[1;31m'
_TERM_GREEN = '\033[1;32m'
_TERM_YELLOW = '\033[1;33m'
_TERM_EMPTY = '\033[0m'

_LOG_PATTERN = r'\d\d:\d\d\s+([(\+\d+\s+)|(~\d+\s+)|(\-\d+\s+)]+):\s+(.*)'


class Target:
    """A Tizen device that can run Flutter applications."""

    def __init__(self, name, platform, id=None):
        self.name = name
        self.platform = platform
        self.id = id
        self.target_tuple = (self.platform, self.name, self.id)

    def run_integration_test(self, plugin_name, directory, timeout):
        """Runs integration test in the given directory.
        
        Args:
            plugin_name (str): The name of the testing plugin.
            directory (str): The path to the directory in which to perform 
                             integration test.
            timeout (int): Time limit in seconds before cancelling the test.

        Returns:
            TestResult: The result of the integration test.
        """
        is_timed_out = False
        process = subprocess.Popen(
            f'flutter-tizen -d {self.id} test integration_test',
            shell=True,
            cwd=directory,
            universal_newlines=True,
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE)
        last_line = ''
        start = time.time()
        for line in process.stdout:
            match = re.search(_LOG_PATTERN, line)
            last_match = re.search(_LOG_PATTERN, last_line)
            if match and last_match and last_match.group(2) == match.group(2):
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
            return TestResult.fail(plugin_name, self.target_tuple, [
                '''Timeout expired. The test may need more time to finish.
If you expect the test to finish before timeout, check if the tests 
require device screen to be awake or if they require manually 
clicking the UI button for permissions.'''
            ])
        if last_line.strip() == 'No tests ran.':
            # This message occurs when the integration test file exists,
            # but no actual test code is written in it.

            return TestResult.fail(plugin_name, self.target_tuple, [
                'Missing integration tests \
                    (use --exclude if this is intentional).'
            ])
        elif last_line.strip().startswith('No devices found'):
            return TestResult.fail(plugin_name, self.target_tuple,
                                   ['Device was disconnected during test.'])

        match = re.search(_LOG_PATTERN, last_line.strip())
        if not match:
            # To protect from illegal log messages that may be introduced in the
            # future.
            raise Exception('Log message is not parsed correctly.')

        # In some cases, the command returns 0 for failed cases,
        # so we check again with the last log message.
        if match.group(2) == 'All tests passed!':
            return TestResult.success(plugin_name, self.target_tuple)
        else:
            # match.group(2) == 'Some tests failed.'
            return TestResult.fail(plugin_name, self.target_tuple, [
                'flutter-tizen test integration_test failed, \
                    see the output above for details.'
            ])


class TargetManager:
    """A manager class that finds and keep tracks of Tizen targets."""

    def __init__(self):
        self.target_per_id = {}
        self.targets_per_platform = defaultdict(list)
        self._find_all_targets()

    def exists_platform(self, platform):
        return len(self.targets_per_platform[platform]) > 0

    def exists_id(self, id):
        return id in self.target_per_id

    def get_by_platform(self, platform):
        return self.targets_per_platform[platform]

    def get_by_id(self, id):
        return self.target_per_id[id]

    def platforms(self):
        return self.targets_per_platform.keys()

    def _find_all_targets(self):
        completed_process = subprocess.run('sdb devices',
                                           shell=True,
                                           cwd='.',
                                           universal_newlines=True,
                                           stderr=subprocess.PIPE,
                                           stdout=subprocess.PIPE)
        if completed_process.returncode != 0:
            raise Exception('sdb failure.')

        lines = completed_process.stdout.rstrip().split('\n')
        for line in lines[1:]:
            tokens = re.split('[\t ]', line)
            id = tokens[0]
            name = tokens[-1]
            completed_process = subprocess.run(f'sdb -s {id} capability',
                                               shell=True,
                                               cwd='.',
                                               universal_newlines=True,
                                               stderr=subprocess.PIPE,
                                               stdout=subprocess.PIPE)
            if completed_process.returncode != 0:
                print(f'capability failure for target {id}.')
                continue
            device_profile, tizen_version = self._parse_target_info(
                completed_process.stdout)
            if not device_profile or not tizen_version:
                print(f'''Cannot extract device profile or 
Tizen version information from target {id}.
device_profile: {device_profile}
tizen_version: {tizen_version}''')
                continue

            platform = f'{device_profile}-{tizen_version}'
            target = Target(name, platform, id)
            self.target_per_id[id] = target
            self.targets_per_platform[platform].append(target)

    def _parse_target_info(self, capability_info):
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


class TestResult:
    """A class that specifies the result of a plugin integration test.

    Attributes:
        run_state: The result of the test. Can be either succeeded for failed.
        details: A list of details about the test result. (e.g. reasons for failure.)
    """

    def __init__(self, plugin_name, run_state, target='', details=[]):
        self.plugin_name = plugin_name
        self.target = target
        self.run_state = run_state
        self.details = details

    @classmethod
    def success(cls, plugin_name, target):
        return cls(plugin_name, 'succeeded', target)

    @classmethod
    def fail(cls, plugin_name, target='', errors=[]):
        return cls(plugin_name, 'failed', target, details=errors)


def set_subparser(subparsers):
    parser = subparsers.add_parser(
        'test',
        help='Run integration test',
        description='''Runs integration test on specified plugins.
By default it will run integration tests on all plugins under packages with 
all connected targets.''')
    command_utils.set_parser_arguments(parser,
                                       plugins=True,
                                       exclude=True,
                                       run_on_changed_packages=True,
                                       base_sha=True,
                                       timeout=True,
                                       command='test')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--platforms',
                       type=str,
                       nargs='*',
                       default=[],
                       help='''Run integration test on all connected targets 
that satisfy <device_profile>-<platform_version> (ex: wearable-5.5, tv-6.0).
The selected targets will be used for all plugins, if you wish to run different 
targets for each plugin, use the --recipe option instead.
''')
    parser.add_argument('--recipe',
                        type=str,
                        default='',
                        help='''The recipe file path. A recipe refers to a 
yaml file that defines a list of target platforms to test for each plugin. 
Pass this file if you want to select specific target platform for different
plugins. Note that recipe does not select which plugins to test(that is covered
by the --plugins option), it only defines which target platform to test
for certain plugins if those plugins are being tested.
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

    target_manager = TargetManager()
    if not platforms:
        platforms.extend(target_manager.platforms())
        if not platforms:
            return [
                TestResult.fail(plugin_name,
                                errors=['Cannot find any connected targets.'])
            ]

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

        for platform in platforms:
            if not target_manager.exists_platform(platform):
                test_results.append(
                    TestResult.fail(
                        plugin_name, platform,
                        [f'Test runner cannot find any {platform} targets.']))
                continue
            targets = target_manager.get_by_platform(platform)
            for target in targets:
                result = target.run_integration_test(plugin_name, example_dir,
                                                     timeout)
                test_results.append(result)

    finally:
        subprocess.run('flutter-tizen clean',
                       shell=True,
                       cwd=example_dir,
                       stdout=open(os.devnull, 'wb'))

    return test_results


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
        platforms = args.platforms
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
            f'{color}{result.run_state.upper()}: {result.plugin_name} {result.target}{_TERM_EMPTY}'
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
