#!/usr/bin/env python3

import argparse
import subprocess
import os
from time import sleep, time


def set_parser_arguments(parser,
                       plugins=False, exclude=False, run_on_changed_packages=False, base_sha=False, timeout=False, command=''):
    if plugins:
        parser.add_argument(
            '--plugins',
            type=str,
            nargs='*',
            default=[],
            help=f'Specifies which plugins to {command}. \
            If it is not specified and --run-on-changed-packages is also not specified, \
            then it will include every plugin under packages. \
            If both flags are specified, then --run-on-changed-packages is ignored.')

    if exclude:
        parser.add_argument('--exclude',
                            type=str,
                            nargs='*',
                            default=[],
                            help=f'Exclude plugins from {command}.')

    if run_on_changed_packages:
        parser.add_argument(
            '--run-on-changed-packages',
            default=False,
            action='store_true',
            help=f'Run the {command} on changed plugins.')

    if base_sha:
        parser.add_argument(
            '--base-sha',
            type=str,
            default='',
            help='The base sha used to determine git diff. This is useful when \
            --run-on-changed-packages is specified. If not specified, \
                merge-base is used as base sha.')

    if timeout:
        parser.add_argument(
            '--timeout',
            type=int,
            default=120,
            help=f'Timeout limit of each {command} in seconds. \
            Default is 120 seconds.')


def get_changed_plugins(packages_dir, base_sha=''):
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
        f'git diff --name-only {base_sha} HEAD',
        shell=True,
        cwd=packages_dir,
        encoding='utf-8',
        stdout=subprocess.PIPE).stdout.strip().splitlines()

    changed_plugins = []
    for changed_file in changed_files:
        path_segments = changed_file.split('/')
        if 'packages' not in path_segments:
            continue
        index = path_segments.index('packages')
        if index < len(path_segments) and path_segments[index + 1] not in changed_plugins:
            changed_plugins.append(path_segments[index + 1])

    return list(set(changed_plugins))


def get_target_plugins(packages_dir, plugins=[], exclude=[], run_on_changed_packages=False, base_sha=''):
    existing_plugins = os.listdir(packages_dir)
    for plugin in plugins:
        if plugin not in existing_plugins:
            print(f'{plugin} package does not exist, ignoring input...')

    plugin_names = []
    if len(plugins) == 0 and run_on_changed_packages:
        plugin_names = get_changed_plugins(packages_dir, base_sha)
    else:
        for plugin_name in existing_plugins:
            if not os.path.isdir(os.path.join(packages_dir, plugin_name)):
                continue
            if len(plugins) > 0 and plugin_name not in plugins:
                continue
            plugin_names.append(plugin_name)

    excluded_plugins = []
    testing_plugins = []
    for plugin_name in plugin_names:
        if plugin_name in exclude:
            excluded_plugins.append(plugin_name)
        else:
            testing_plugins.append(plugin_name)
    return testing_plugins, excluded_plugins


def get_package_dir():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '../../packages'))
