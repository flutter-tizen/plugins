#!/usr/bin/env python3

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
