#!/usr/bin/env python3

import re
import subprocess
import sys
import os
import commands.command_utils as command_utils


def parse_args(args):
    parser = command_utils.get_options_parser(
        plugins=True, exclude=True, run_on_changed_packages=True, base_sha=True, command='build')
    return parser.parse_args(args)


def _build_examples(plugin):
    name = plugin[plugin.rfind('/')+1:]
    print(f'============= Build {name} ... =============')
    example_dir = os.path.join(plugin, 'example')
    subprocess.run('flutter-tizen pub get',
                   shell=True,
                   cwd=example_dir)

    completed_process = subprocess.run('flutter-tizen build tpk --device-profile wearable -v',
                                       shell=True,
                                       cwd=example_dir,
                                       universal_newlines=True)
    if completed_process.returncode == 0:
        print(f'============= Succeed to build {name} =============')
        return True
    else:
        print(f'============= Failed to build {name} =============')
        return False


def run_build_examples(argv):
    args = parse_args(argv)
    packages_dir = command_utils.get_package_dir()
    target_plugins, _ = command_utils.get_target_plugins(
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
