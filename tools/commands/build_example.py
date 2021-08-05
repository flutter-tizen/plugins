#!/usr/bin/env python3

import re
import subprocess
import sys
import os
import commands.command_utils as command_utils


def parse_args(args):
    parser = command_utils.get_options_parser(plugins=True, exclude=True, run_on_changed_packages=True, base_sha=True)
    return parser.parse_args(args)


def _build_examples(plugin):
    print(f'============= Building for {plugin} =============')
    example_dir = os.path.join(plugin, 'example')
    subprocess.run('flutter-tizen pub get',
                   shell=True,
                   cwd=example_dir,
                   stderr=subprocess.PIPE,
                   stdout=subprocess.PIPE)

    completed_process = subprocess.run('flutter-tizen build tpk --device-profile wearable -v',
                                       shell=True,
                                       cwd=example_dir,
                                       universal_newlines=True,
                                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    print(completed_process.stdout)
    if completed_process.returncode == 0:
        print(f'============= Succeed to build {plugin} =============')
        return True
    else:
        print(f'============= Failed to build {plugin} =============')
        return False


def run_build_examples(argv):
    args = parse_args(argv)
    packages_dir = command_utils.get_package_dir()
    target_plugins, exclude_plugins = command_utils.get_target_plugins(
        packages_dir, plugins=args.plugins, exclude=args.exclude, run_on_changed_packages=args.run_on_changed_packages,
        base_sha=args.base_sha)
    results = []
    for plugin in target_plugins:
        result = _build_examples(os.path.join(packages_dir, plugin))
        results.append(result)

    if False not in results:
        exit(0)
    else:
        exit(1)


if __name__ == '__main__':
    run_build_examples(sys.argv)
