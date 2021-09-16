#!/usr/bin/env python3

import sys
import argparse

from commands import check_tidy
from commands import integration_test
from commands import build_example
from commands import print_plugins
from commands import command_utils


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

    try:
        if args.subcommand == 'tidy':
            check_tidy.run_check_tidy(args)
        elif args.subcommand == 'test':
            integration_test.run_integration_test(args)
        elif args.subcommand == 'build':
            build_example.run_build_examples(args)
        elif args.subcommand == 'plugins':
            print_plugins.run_print_plugins(args)
    except Exception as e:
        print(e)
        exit(1)
