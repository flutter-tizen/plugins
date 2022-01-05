#!/usr/bin/env python3
"""CLI tool that manages multiple plugin packages in this repository.
It is mainly intended to run in CI systems. Each subcommand has the following
dependent executables and the system must be able to find their correct paths.

- check_tidy: clang-format-11
- integration_test: flutter-tizen, sdb, em-cli
- build_example: flutter-tizen
"""

import sys
import argparse

from commands import (
    check_tidy,
    integration_test,
    build_example,
    print_plugins,
)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='subcommand')
    check_tidy.set_subparser(subparsers)
    integration_test.set_subparser(subparsers)
    build_example.set_subparser(subparsers)
    print_plugins.set_subparser(subparsers)

    args = parser.parse_args(sys.argv[1:])
    if not args.subcommand:
        parser.print_help()
        exit(1)

    args.func(args)
