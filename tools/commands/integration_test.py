#!/usr/bin/env python3

"""A script that helps running integration tests for multiple Tizen plugins.

To run integrations tests for all plugins under packages/, 
run tools/run_integration_test.py in the root of this repository.

Here are some of the common use cases. Assume packages contain plugins 
a, b, c, d, and e.
- running specific packages:
tools/run_integration_test.py --plugins a b c # runs a, b, c
- excluding some packages:
tools/run_integration_test.py --exclude a b c # runs d, e
- exclude has precedence:
tools/run_integration_test.py --plugins a b c --exclude b # runs a c
"""

import argparse
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

_DEFAULT_TEST_TARGET = 'wearable-5.5'

class TestResult:
    """A class that specifies the result of a plugin integration test.

    Attributes:
        run_state: The result of the test. Can be either succeeded for failed.
        details: A list of details about the test result. (e.g. reasons for failure.)
    """

    def __init__(self, plugin_name, run_state, test_target='', details=[]):
        self.plugin_name = plugin_name
        self.test_target = test_target
        self.run_state = run_state
        self.details = details

    @classmethod
    def success(cls, plugin_name, test_target):
        return cls(plugin_name, 'succeeded', test_target=test_target)

    @classmethod
    def fail(cls, plugin_name, test_target, errors=[]):
        return cls(plugin_name, 'failed', test_target=test_target, details=errors)


def parse_args(args):
    parser = command_utils.get_options_parser(
        plugins=True, exclude=True, run_on_changed_packages=True, base_sha=True, timeout=True, command='test')
    parser.add_argument(
        '--recipe',
        type=str,
        default='',
        help='''
The recipe file path. A recipe refers to a yaml file that defines a list of test targets for plugins.
Passing this file will allow the tool to test with specified targets instead of the default target, wearable-5.5.
(
plugins:
  a: [wearable-5.5, tv-6.0]
  b: [mobile-6.0]
  c: [wearable-4.0]
)
''')

    return parser.parse_args(args)


def _integration_test(plugin_dir, test_targets, timeout):
    """Runs integration test in the example package for plugin_dir

    Currently the tools assumes that there's only one example package per plugin.

    Args:
        plugin_dir (str): The path to a single plugin directory.
        test_targets (List[str]): A list of testing targets.
        timeout (int): Time limit in seconds before cancelling the test.

    Returns:
        TestResult: The result of the plugin integration test.
    """
    plugin_name = os.path.basename(plugin_dir)

    if not test_targets:
        # (TODO: HakkyuKim) Improve logic for setting default targets.
        test_targets.append(_DEFAULT_TEST_TARGET)

    example_dir = os.path.join(plugin_dir, 'example')
    if not os.path.isdir(example_dir):
        return [TestResult.fail(plugin_name, errors=[
            'Missing example directory (use --exclude if this is intentional).'
        ])]

    pubspec_path = os.path.join(example_dir, 'pubspec')
    if not os.path.isfile(f'{pubspec_path}.yaml') and not os.path.isfile(
            f'{pubspec_path}.yml'):
        # TODO: Support multiple example packages.
        return [TestResult.fail(plugin_name, errors=['Missing pubspec file in example directory'])]

    integration_test_dir = os.path.join(example_dir, 'integration_test')
    if not os.path.isdir(integration_test_dir) or not os.listdir(
            integration_test_dir):
        return [TestResult.fail(plugin_name, errors=[
            'Missing integration tests (use --exclude if this is intentional).'
        ])]

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
    target_table = _get_target_table()

    for test_target in test_targets:
        if test_target not in target_table:
            test_results.append(TestResult.fail(plugin_name, test_target, [
                                f'Test runner cannot find target {test_target}.']))
            continue

        is_timed_out = False
        process = subprocess.Popen(f'flutter-tizen -d {target_table[test_target]} test integration_test',
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
            errors.append("""Timeout expired. The test may need more time to finish.
    If you expect the test to finish before timeout, check if the tests 
    require device screen to be awake or if they require manually 
    clicking the UI button for permissions.""")
            test_results.append(TestResult.fail(
                plugin_name, test_target, errors=errors))
            continue
        if last_line.strip() == 'No tests ran.':
            test_results.append(TestResult.fail(
                plugin_name, test_target, ['No tests ran.']))
            continue
        elif last_line.strip().startswith('No devices found'):
            test_results.append(TestResult.fail(plugin_name, test_target, [
                'The runner cannot find any devices to run tests.'
            ]))
            continue

        match = re.search(_LOG_PATTERN, last_line.strip())
        if not match:
            test_results.append(TestResult.fail(plugin_name, test_target,
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
            test_results.append(TestResult.success(plugin_name, test_target))
        else:
            test_results.append(TestResult.fail(
                plugin_name, test_target, errors=errors))

    return test_results


def _get_target_table():

    def _parse_target_info(capability_info: str):
        capability_info.rstrip()
        profile_name = ''
        platform_version = ''
        for line in capability_info.split('\n'):
            tokens = line.split(':')
            if(tokens[0] == 'profile_name'):
                profile_name = tokens[1]
            elif(tokens[0] == 'platform_version'):
                platform_version = tokens[1]
        return profile_name, platform_version

    completed_process = subprocess.run('sdb devices',
                                       shell=True,
                                       cwd='.',
                                       universal_newlines=True,
                                       stderr=subprocess.PIPE,
                                       stdout=subprocess.PIPE)

    table = {}
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
            profile, platform_version = _parse_target_info(
                completed_process.stdout)
            if not profile or not platform_version:
                print(f'''
Cannot extract {profile} or {platform_version} information from device {id}
profile: {profile}
platform_version: {platform_version}
''')
            if f'{profile}-{platform_version}' in table:
                print(f'Multiple targets of {profile}-{platform_version} found. Replacing {table[{profile}-{platform_version}]} to {id}...')
            table[f'{profile}-{platform_version}'] = id
    return table


def run_integration_test(argv):
    args = parse_args(argv)

    test_targets = {}
    if args.recipe:
        if not os.path.isfile(args.recipe):
            print(f'The recipe file {args.recipe} does not exist.')
            exit(1)
        with open(args.recipe) as f:
            try:
                test_targets = yaml.load(
                    f.read(), Loader=yaml.FullLoader)['plugins']
            except yaml.parser.ParserError:
                print(f'The recipe file {args.recipe} is not a valid yaml file.')
                exit(1)

    packages_dir = command_utils.get_package_dir()
    testing_plugins, excluded_plugins = command_utils.get_target_plugins(
        packages_dir, plugins=args.plugins, exclude=args.exclude, run_on_changed_packages=args.run_on_changed_packages,
        base_sha=args.base_sha)

    test_num = 0
    total_plugin_num = len(testing_plugins)
    results = []
    for testing_plugin in testing_plugins:
        test_num += 1
        print(
            f'============= Testing for {testing_plugin} ({test_num}/{total_plugin_num}) ============='
        )
        test_targets_list = []
        if testing_plugin in test_targets:
            test_targets_list = test_targets[testing_plugin]

        results.extend(_integration_test(
            os.path.join(packages_dir, testing_plugin), test_targets_list, args.timeout))

    print(f'============= TEST RESULT =============')
    failed_plugins = []
    for result in results:
        color = _TERM_GREEN
        if result.run_state == 'failed':
            color = _TERM_RED

        print(
            f'{color}{result.run_state.upper()}: {result.plugin_name} {result.test_target}{_TERM_EMPTY}')
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


if __name__ == "__main__":
    run_integration_test(sys.argv)
