#!/usr/bin/env python3
import argparse
import subprocess
import sys
import os

_INDENTATION = '    '
_TERM_RED = '\033[1;31m'
_TERM_GREEN = '\033[1;32m'
_TERM_YELLOW = '\033[1;33m'
_TERM_EMPTY = '\033[0m'


class PluginResult:
    def __init__(self, run_state, details=[]):
        self.run_state = run_state
        self.details = details

    @classmethod
    def success(cls):
        return cls('succeeded')

    # Skip may be used for testing on unsupported device profiles.
    @classmethod
    def skip(cls, reason):
        return cls('skipped', details=[reason])

    @classmethod
    def fail(cls, errors=[]):
        return cls('failed', details=errors)


def parse_args(args):
    parser = argparse.ArgumentParser(
        description='A script to run multiple tizen plugin driver tests.')

    parser.add_argument('--plugins', type=str, nargs='*', default=[],
                        help='Specifies which plugins to test. If it is not specified and --run-on-changed-packages is also not specified, then it will include every plugin under packages.')
    parser.add_argument('--exclude', type=str, nargs='*', default=[],
                        help='Exclude plugins from test. Excluding plugins also works with --run-on-changed-packages flag.')
    parser.add_argument('--run-on-changed-plugins',
                        default=False, action='store_true', help='Run the test on changed plugins. If --plugins is specified, this flag is ignored.')
    parser.add_argument('--base-sha', type=str, default='',
                        help='The base sha used to determine git diff. This is useful when --run-on-changed-packages is specified. If not specified, merge-base is used as base sha.')

    return parser.parse_args(args)


def drive_example(plugin_dir):
    example_dir = os.path.join(plugin_dir, 'example')
    if not os.path.isdir(example_dir):
        return PluginResult.fail('Missing example directory (use --exclude if this is intentional).')

    pubspec_path = os.path.join(example_dir, 'pubspec')
    if not os.path.isfile(f'{pubspec_path}.yaml') and not os.path.isfile(f'{pubspec_path}.yml'):
        return PluginResult.fail('Missing pubspec file in example directory')

    driver_paths = []
    driver_dir = os.path.join(example_dir, 'test_driver')
    if os.path.isdir(driver_dir):
        for driver_name in os.listdir(driver_dir):
            driver_path = os.path.join(driver_dir, driver_name)
            if os.path.isfile(driver_path) and driver_name.endswith('_test.dart'):
                driver_paths.append(driver_path)

    test_target_paths = []
    integration_test_dir = os.path.join(example_dir, 'integration_test')
    if os.path.isdir(integration_test_dir):
        for test_target_name in os.listdir(integration_test_dir):
            test_target_path = os.path.join(
                integration_test_dir, test_target_name)
            if os.path.isfile(test_target_path) and test_target_name.endswith('_test.dart'):
                test_target_paths.append(test_target_path)

    errors = []
    tests_ran = False
    if not driver_paths:
        print(f'{_INDENTATION}No driver tests found for {os.path.basename(plugin_dir)}')
        errors.append(f'No driver files for {os.path.basename(plugin_dir)}')
    elif not test_target_paths:
        driver_relpaths = [os.path.relpath(driver_path, os.path.dirname(
            plugin_dir)) for driver_path in driver_paths]
        print(
            f'Found {driver_relpaths}, but no integration_test/*_test.dart files.')
        errors.append(f'No test files for {driver_relpaths}')

    for driver_path in driver_paths:
        for test_target_path in test_target_paths:
            tests_ran = True
            print(f'{_INDENTATION}Running --driver {os.path.basename(driver_path)} --target {os.path.basename(test_target_path)}')
            completed_process = subprocess.run('flutter-tizen pub get',
                                               shell=True, cwd=example_dir)
            if completed_process.returncode != 0:
                if not completed_process.stderr:
                    errors.append(
                        'pub get failed, see the output above for details.')
                else:
                    errors.append(completed_process.stderr)
                continue
            try:
                completed_process = subprocess.run(
                    f'flutter-tizen drive --driver={driver_path} --target={test_target_path}', shell=True, cwd=example_dir, timeout=300)
                if completed_process.returncode != 0:
                    if not completed_process.stderr:
                        errors.append(
                            'flutter-tizen drive failed, see the output above for details.')
                    else:
                        errors.append(completed_process.stderr)
                continue
            except subprocess.TimeoutExpired:
                errors.append('Timeout expired')

    if not tests_ran:
        print(
            f'{_INDENTATION}No driver tests were run for {os.path.basename(plugin_dir)}.')
        errors.append('No tests ran (use --exclude if this is intentional).')

    return PluginResult.success() if len(errors) == 0 else PluginResult.fail(errors)


def main(argv):
    args = parse_args(argv[1:])

    packages_dir = os.path.abspath(os.path.join(
        os.path.dirname(__file__), '../packages'))

    plugin_names = []
    if len(args.plugins) == 0 and args.run_on_changed_plugins:
        base_sha = args.base_sha
        if base_sha == '':
            base_sha = subprocess.run('git merge-base --fork-point FETCH_HEAD HEAD', shell=True,
                                      cwd=packages_dir, encoding='utf-8', stdout=subprocess.PIPE).stdout.strip()
            if base_sha == '':
                base_sha = subprocess.run(
                    'git merge-base FETCH_HEAD HEAD', shell=True, cwd=packages_dir, encoding='utf-8', stdout=subprocess.PIPE).stdout.strip()
        changed_files = subprocess.run(
            f'git diff --name-only {base_sha} HEAD', shell=True, cwd=packages_dir, encoding='utf-8', stdout=subprocess.PIPE).stdout.strip().splitlines()

        changed_plugins = []
        for changed_file in changed_files:
            relpath = os.path.relpath(
                changed_file, start=os.path.dirname(packages_dir))
            path_segments = relpath.split('/')
            if 'packages' not in path_segments:
                continue
            index = path_segments.index('packages')
            if index < len(path_segments):
                changed_plugins.append(path_segments[index + 1])
        plugin_names = list(set(changed_plugins))
    else:
        for plugin_name in os.listdir(packages_dir):
            plugin_dir = os.path.join(packages_dir, plugin_name)
            if not os.path.isdir(plugin_dir):
                continue
            if len(args.plugins) > 0 and plugin_name not in args.plugins:
                continue
            plugin_names.append(plugin_name)

    excluded_plugins = [
        plugin_name for plugin_name in plugin_names if plugin_name in args.exclude]
    plugin_names = [
        plugin_name for plugin_name in plugin_names if plugin_name not in args.exclude]

    test_num = 0
    total_plugin_num = len(plugin_names)
    results = {}
    for plugin_name in plugin_names:
        plugin_dir = os.path.join(packages_dir, plugin_name)
        test_num += 1
        print(
            f'{_INDENTATION}Testing for {plugin_name} ({test_num}/{total_plugin_num})')
        results[plugin_name] = drive_example(plugin_dir)

    print(f'============= TEST RESULT =============')
    failed_plugins = []
    for plugin_name, result in results.items():
        color = _TERM_GREEN
        if result.run_state == 'failed':
            color = _TERM_RED

        print(
            f'{color}{result.run_state.upper()}: {plugin_name}{_TERM_EMPTY}')
        if result.run_state != 'succeeded':
            print(f'{_INDENTATION}DETAILS: {result.details}')
        if result.run_state == 'failed':
            failed_plugins.append(plugin_name)

    exit_code = 0
    if len(failed_plugins) > 0:
        print(f'{_TERM_RED}============= TEST FAILED ============={_TERM_EMPTY}')
        for failed_plugin in failed_plugins:
            print(
                f'FAILED: {failed_plugin} DETAILS: {results[failed_plugin].details}')
        exit_code = 1
    elif total_plugin_num == 0:
        print('No tests ran.')
    else:
        print(f'{_TERM_GREEN}All tests passed.{_TERM_EMPTY}')

    for excluded_plugin in excluded_plugins:
        print(f'{_TERM_YELLOW}EXCLUDED: {excluded_plugin}{_TERM_EMPTY}')
    exit(exit_code)


if __name__ == "__main__":
    main(sys.argv)
