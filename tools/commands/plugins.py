#!/usr/bin/env python3

import sys

import commands.command_utils as command_utils


def parse_args(args):
    parser = command_utils.get_options_parser(run_on_changed_packages=True, base_sha=True)
    return parser.parse_args(args)


def run_plugins(argv):
    args = parse_args(argv)
    packages_dir = command_utils.get_package_dir()
    target_plugins, exclude_plugins = command_utils.get_target_plugins(
        packages_dir, run_on_changed_packages=args.run_on_changed_packages, base_sha=args.base_sha)
    print(target_plugins)


if __name__ == '__main__':
    run_plugins(sys.argv)
