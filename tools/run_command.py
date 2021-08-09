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
commands["tidy"] = run_check_tidy
commands["test"] = run_integration_test
commands["build"] = run_build_examples
commands["plugins"] = run_print_plugins

if __name__ == "__main__":
    try:
        commands[sys.argv[1]](sys.argv[2:])
    except Exception as e:
        print(e)
