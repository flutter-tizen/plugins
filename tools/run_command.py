#!/usr/bin/env python3

import sys

from commands import check_tidy
from commands import integration_test
from commands import build_example
from commands import print_plugins


# Check tidy
def run_check_tidy(argv):
    check_tidy.run_check_tidy(argv)


# Excute integration_test
def run_integration_test(argv):
    integration_test.run_integration_test(argv)


# Build example app
def run_build_examples(argv):
    build_example.run_build_examples(argv)


# Print plugin list
def run_print_plugins(arv):
    print_plugins.run_print_plugins(arv)


commands = {}
commands['tidy'] = {'func': run_check_tidy, 'info': 'Check and update format for C++ files'}
commands['test'] = {'func': run_integration_test, 'info': 'Run integration test'}
commands['build'] = {'func': run_build_examples, 'info': 'Build examples of plugin'}
commands['plugins'] = {'func': run_print_plugins, 'info': 'Print plugins list'}


def print_usage():
    print('usage: run_command.py [command] ')
    print('command lists:')
    for k, v in commands.items():
        print(f'    { k } : { v["info"] }')


if __name__ == "__main__":
    try:
        commands[sys.argv[1]]['func'](sys.argv[2:])
    except Exception as e:
        print_usage()
        exit(1)
