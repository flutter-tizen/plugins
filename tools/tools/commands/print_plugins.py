import commands.command_utils as command_utils


def set_subparser(subparsers):
    parser = subparsers.add_parser('plugins', help='Print plugins list')
    command_utils.set_parser_arguments(parser,
                                       run_on_changed_packages=True,
                                       base_sha=True,
                                       command='print plugins')
    parser.set_defaults(func=run_print_plugins)


def run_print_plugins(args):
    packages_dir = command_utils.get_package_dir()

    target_plugins, _ = command_utils.get_target_plugins(
        packages_dir,
        run_on_changed_packages=args.run_on_changed_packages,
        base_sha=args.base_sha)
    for target_plugin in target_plugins:
        print(target_plugin)
