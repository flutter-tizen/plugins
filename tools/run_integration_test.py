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
import subprocess
import sys
import os
import time
import re

_TERM_RED = '\033[1;31m'
_TERM_GREEN = '\033[1;32m'
_TERM_YELLOW = '\033[1;33m'
_TERM_EMPTY = '\033[0m'

_LOG_PATTERN = r'\d\d:\d\d\s+([(\+\d+\s+)|(~\d+\s+)|(\-\d+\s+)]+):\s+(.*)'


class TestResult:
    """A class that specifies the result of a plugin integration test.

    Attributes:
        run_state: The result of the test. Can be either succeeded for failed.
        details: A list of details about the test result. (e.g. reasons for failure.)
    """

    def __init__(self, run_state, details=[]):
        self.run_state = run_state
        self.details = details

    @classmethod
    def success(cls):
        return cls('succeeded')

    @classmethod
    def fail(cls, errors=[]):
        return cls('failed', details=errors)


def parse_args(args):
    parser = argparse.ArgumentParser(
        description='A script to run multiple Tizen plugin driver tests.')

    parser.add_argument(
        '--plugins',
        type=str,
        nargs='*',
        default=[],
        help='Specifies which plugins to test. If it is not specified and \
            --run-on-changed-packages is also not specified, \
                then it will include every plugin under packages.')
    parser.add_argument('--exclude',
                        type=str,
                        nargs='*',
                        default=[],
                        help='Exclude plugins from test.')
    parser.add_argument(
        '--run-on-changed-packages',
        default=False,
        action='store_true',
        help='Run the test on changed plugins. If --plugins is specified, \
            this flag is ignored.')
    parser.add_argument(
        '--base-sha',
        type=str,
        default='',
        help='The base sha used to determine git diff. This is useful when \
            --run-on-changed-packages is specified. If not specified, \
                merge-base is used as base sha.')
    parser.add_argument(
        '--timeout',
        type=int,
        default=120,
        help='Timeout limit of each integration test in seconds. \
            Default is 120 seconds.')

    return parser.parse_args(args)


def run_integration_test(plugin_dir, timeout):
    """Runs integration test in the example package for plugin_dir

    Currently the tools assumes that there's only one example package per plugin.

    Args:
        plugin_dir (str): The path to a single plugin directory.
        timeout (int): Time limit in seconds before cancelling the test.

    Returns:
        TestResult: The result of the plugin integration test.
    """
    example_dir = os.path.join(plugin_dir, 'example')
    if not os.path.isdir(example_dir):
        return TestResult.fail([
            'Missing example directory (use --exclude if this is intentional).'
        ])

    pubspec_path = os.path.join(example_dir, 'pubspec')
    if not os.path.isfile(f'{pubspec_path}.yaml') and not os.path.isfile(
            f'{pubspec_path}.yml'):
        # TODO: Support multiple example packages.
        return TestResult.fail(['Missing pubspec file in example directory'])

    integration_test_dir = os.path.join(example_dir, 'integration_test')
    if not os.path.isdir(integration_test_dir) or not os.listdir(
            integration_test_dir):
        return TestResult.fail([
            'Missing integration tests (use --exclude if this is intentional).'
        ])

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
        return TestResult.fail(errors)

    is_timed_out = False
    process = subprocess.Popen('flutter-tizen test integration_test',
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
        return TestResult.fail(errors)
    if last_line.strip() == 'No tests ran.':
        return TestResult.fail(['No tests ran.'])
    elif last_line.strip().startswith('No devices found'):
        return TestResult.fail([
            'The runner cannot find any devices to run tests. Check if the hosted test server has connections to Tizen devices.'
        ])

    match = re.search(_LOG_PATTERN, last_line.strip())
    if not match:
        return TestResult.fail(['Log message is not formatted correctly.'])

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
        return TestResult.success()
    else:
        return TestResult.fail(errors)


def main(argv):
    args = parse_args(argv[1:])

    packages_dir = os.path.abspath(
        os.path.join(os.path.dirname(__file__), '../packages'))
    existing_plugins = os.listdir(packages_dir)
    for plugin in args.plugins:
        if plugin not in existing_plugins:
            print(f'{plugin} package does not exist, ignoring input...')

    plugin_names = []
    if len(args.plugins) == 0 and args.run_on_changed_packages:
        base_sha = args.base_sha
        if base_sha == '':
            base_sha = subprocess.run(
                'git merge-base --fork-point FETCH_HEAD HEAD',
                shell=True,
                cwd=packages_dir,
                encoding='utf-8',
                stdout=subprocess.PIPE).stdout.strip()
            if base_sha == '':
                base_sha = subprocess.run(
                    'git merge-base FETCH_HEAD HEAD',
                    shell=True,
                    cwd=packages_dir,
                    encoding='utf-8',
                    stdout=subprocess.PIPE).stdout.strip()

        changed_files = subprocess.run(
            f'git diff {base_sha} HEAD',
            shell=True,
            cwd=packages_dir,
            encoding='utf-8',
            stdout=subprocess.PIPE).stdout.strip().splitlines()

        changed_plugins = []
        for changed_file in changed_files:
            relpath = os.path.relpath(changed_file, start=packages_dir)
            path_segments = relpath.split('/')
            if 'packages' not in path_segments:
                continue
            index = path_segments.index('packages')
            if index < len(path_segments):
                changed_plugins.append(path_segments[index + 1])
        plugin_names = list(set(changed_plugins))
    else:
        for plugin_name in existing_plugins:
            if not os.path.isdir(os.path.join(packages_dir, plugin_name)):
                continue
            if len(args.plugins) > 0 and plugin_name not in args.plugins:
                continue
            plugin_names.append(plugin_name)

    excluded_plugins = []
    testing_plugins = []
    for plugin_name in plugin_names:
        if plugin_name in args.exclude:
            excluded_plugins.append(plugin_name)
        else:
            testing_plugins.append(plugin_name)

    test_num = 0
    total_plugin_num = len(testing_plugins)
    results = {}
    for testing_plugin in testing_plugins:
        test_num += 1
        print(
            f'============= Testing for {testing_plugin} ({test_num}/{total_plugin_num}) ============='
        )
        results[testing_plugin] = run_integration_test(
            os.path.join(packages_dir, testing_plugin), args.timeout)

    print(f'============= TEST RESULT =============')
    failed_plugins = []
    for testing_plugin, result in results.items():
        color = _TERM_GREEN
        if result.run_state == 'failed':
            color = _TERM_RED

        print(
            f'{color}{result.run_state.upper()}: {testing_plugin}{_TERM_EMPTY}')
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
    main(sys.argv)
