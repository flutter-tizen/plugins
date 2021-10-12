"""A subcommand that runs integration tests for multiple Tizen plugins 
under packages. By default, the tool will run integration tests of all plugins
under packages on all connected targets.

Here are some of the common use cases. Assume packages contain plugins 
a, b, c, d, and e.
- testing specific packages:
tools/run_command.py test --plugins a b c # runs a, b, c
- excluding some packages:
tools/run_command.py test --exclude a b c # runs d, e
- exclude has precedence:
tools/run_command.py test --plugins a b c --exclude b # runs a c
- testing on all targets that satisfies wearable-5.5 platform:
tools/run_command.py test --platforms wearable-5.5
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
    """A Tizen device that can run Flutter applications.
    
    A target is a Tizen device (either a physical device or an emulator) that can
    run Flutter applications. Generally targets are connected when physical devices
    are connected to the host PC either with a cable or wirelessly, and when 
    emulators are launched by Tizen SDK's Emulator Manager.

    Each target has three important pieces of information:
    - name: the name of the target.
    - platform: defines the device profile and Tizen version, expressed in
    <device_profile>-<tizen_version> (ex: wearable-5.5, tv-6.0).
    - id: the identifier assigned to a **connected** target.
    """

    def __init__(self, name, platform, id=None):
        self.name = name
        self.platform = platform
        self.device_profile, self.tizen_version = platform.split('-', 1)
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
                'flutter-tizen test integration_test failed, see the output above for details.'
            ])


class TemporaryEmulator(Target):
    """A Tizen emulator that is created temporary by the tool for testing purposes."""

    def __init__(self, name, platform):
        super().__init__(name, platform)
        self._pid = None

    def run_integration_test(self, plugin_name, directory, timeout):
        self.launch()
        result = super().run_integration_test(plugin_name, directory, timeout)
        self.power_off()
        return result

    def _find_id(self):
        completed_process = subprocess.run('sdb devices',
                                           shell=True,
                                           universal_newlines=True,
                                           stderr=subprocess.PIPE,
                                           stdout=subprocess.PIPE)
        if completed_process.returncode != 0:
            raise Exception('sdb failure.')

        lines = completed_process.stdout.rstrip().split('\n')
        for line in lines[1:]:
            tokens = re.split('\s+', line)
            if tokens[-1] == self.name:
                return tokens[0]
        raise Exception(f'Could not find connected target {self.name}')

    def _find_pid(self):
        completed_process = subprocess.run(
            f'ps a | grep emulator-x86_64 | grep {self.name}',
            shell=True,
            universal_newlines=True,
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE)
        if completed_process.returncode != 0:
            raise Exception(f'Could not find pid of target {self.name}')
        pid = completed_process.stdout.strip().split(' ')[0]
        return pid

    def launch(self):
        completed_process = subprocess.run(f'em-cli launch -n {self.name}',
                                           shell=True)
        if completed_process.returncode != 0:
            raise Exception(f'Target {self.name} launch failed.')
        # There's no straightforward way to know when the target is fully
        # launched. The current sleep setting is based on some testing.
        time.sleep(5)
        self.id = self._find_id()
        self._pid = self._find_pid()

    def power_off(self):
        completed_process = subprocess.run(f'kill -9 {self._pid}',
                                           shell=True,
                                           stdout=open(os.devnull, 'wb'))
        if completed_process.returncode != 0:
            raise Exception(f'Target {self.id} power off failed.')
        time.sleep(1)
        self.id = None
        self.pid = None

    def create(self):
        completed_process = subprocess.run(
            f'em-cli create -n {self.name} -p {self._get_tizensdk_platform()}',
            shell=True)
        if completed_process.returncode != 0:
            raise Exception(f'Target {self.name} creation failed.')

    def delete(self):
        completed_process = subprocess.run(f'em-cli delete -n {self.name}',
                                           shell=True)
        if completed_process.returncode != 0:
            raise Exception(f'Target {self.name} deletion failed.')

    def _get_tizensdk_platform(self):
        """Gets the platform name that's understood by the Tizen sdk's em-cli command."""
        if self.device_profile == 'wearable':
            return f'{self.platform}-circle-x86'
        elif self.device_profile == 'tv':
            return f'tv-samsung-{self.tizen_version}-x86'
        elif self.device_profile == 'mobile':
            return f'{self.platform}-x86'
        else:
            raise Exception(
                f'Test target must start with wearable, mobile, or tv. {self.platform} is an unknown test target.'
            )


class TargetManager:
    """A manager class that finds and manages a collection of Tizen targets."""

    def __init__(self):
        self.targets_per_platform = defaultdict(list)

    def __enter__(self):
        self._find_all_targets()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.targets_per_platform.clear()

    def exists_platform(self, platform):
        return len(self.targets_per_platform[platform]) > 0

    def get_by_platform(self, platform):
        return self.targets_per_platform[platform]

    def get_platforms(self):
        return self.targets_per_platform.keys()

    def _find_all_targets(self):
        completed_process = subprocess.run('sdb devices',
                                           shell=True,
                                           universal_newlines=True,
                                           stderr=subprocess.PIPE,
                                           stdout=subprocess.PIPE)
        if completed_process.returncode != 0:
            raise Exception('sdb failure.')

        lines = completed_process.stdout.rstrip().split('\n')
        for line in lines[1:]:
            tokens = re.split('\s+', line)
            id = tokens[0]
            name = tokens[-1]
            completed_process = subprocess.run(f'sdb -s {id} capability',
                                               shell=True,
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


class TemporaryEmulatorManager(TargetManager):
    """A TargetManager for TemporaryEmulators."""

    def __init__(self, platforms):
        super().__init__()
        self.platforms = platforms

    def __enter__(self):
        for platform in self.platforms:
            self._create_emulator(platform)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self._delete_emulators()
        super().__exit__(exc_type, exc_value, traceback)

    def _create_emulator(self, platform):
        device_profile, tizen_version = platform.split('-', 1)
        # Target name valid characters are [A-Za-z0-9-_].
        emulator_name = f'{device_profile}-{tizen_version.replace(".", "_")}-{os.getpid()}'
        emulator = TemporaryEmulator(emulator_name, platform)
        emulator.create()
        self.targets_per_platform[platform].append(emulator)

    def _delete_emulators(self):
        for targets in self.targets_per_platform.values():
            for target in targets:
                target.delete()


class TestResult:
    """A class that specifies the result of a plugin integration test.

    Attributes:
        plugin_name: The name of the tested plugin.
        run_state: The result of the test. Can be either succeeded for failed.
        target: Information of the target that plugin was tested on.
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
    group.add_argument('--recipe',
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
    parser.add_argument(
        '--generate-emulators',
        default=False,
        action='store_true',
        help='''Create and destroy emulators during test. 
Must provide either --platforms or --recipe option to specify which 
platforms to create.''')
    parser.set_defaults(func=run_integration_test)


def _get_target_manager(generate_emulators, platforms):
    if generate_emulators:
        return TemporaryEmulatorManager(platforms)
    else:
        return TargetManager()


def _integration_test(plugin_dir, platforms, timeout, generate_emulators):
    """Runs integration test in the example package for plugin_dir

    Currently the tools assumes that there's only one example package per plugin.

    Args:
        plugin_dir (str): The path to a single plugin directory.
        platforms (List[str]): A list of testing platforms.
        timeout (int): Time limit in seconds before cancelling the test.
        generate_emulators (bool): Whether to create and delete targets 
                                      for test.

    Returns:
        TestResult: The result of the plugin integration test.
    """
    plugin_name = os.path.basename(plugin_dir)

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
        with _get_target_manager(
                generate_emulators,
                platforms,
        ) as target_manager:
            if not platforms:
                platforms.extend(target_manager.get_platforms())
                if not platforms:
                    return [
                        TestResult.fail(
                            plugin_name,
                            errors=['Cannot find any testable targets.'])
                    ]
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
                        TestResult.fail(plugin_name, platform, [
                            f'Test runner cannot find any {platform} targets.'
                        ]))
                    continue
                targets = target_manager.get_by_platform(platform)
                for target in targets:
                    result = target.run_integration_test(
                        plugin_name, example_dir, timeout)
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

    if args.platforms:
        for platform in args.platforms:
            if '-' not in platform:
                print(
                    f'inputs of --platforms must be <device_profile>-<tizen-version> format, {platform}.'
                )
                exit(1)

    if args.generate_emulators and not args.platforms and not args.recipe:
        print(
            '--generate-emulators option must be used with either --platforms or --recipe option.'
        )
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
            _integration_test(
                os.path.join(packages_dir, testing_plugin),
                platforms,
                args.timeout,
                args.generate_emulators,
            ))

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
