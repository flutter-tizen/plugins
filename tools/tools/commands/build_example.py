import subprocess
import os

import commands.command_utils as command_utils


def set_subparser(subparsers):
    parser = subparsers.add_parser('build', help='Build examples of plugin')
    command_utils.set_parser_arguments(parser,
                                       plugins=True,
                                       exclude=True,
                                       run_on_changed_packages=True,
                                       base_sha=True,
                                       command='build')
    parser.set_defaults(func=run_build_examples)


def _build_examples(plugin_dir):
    example_dir = os.path.join(plugin_dir, 'example')
    subprocess.run('flutter-tizen pub get', shell=True, cwd=example_dir)

    completed_process = subprocess.run(
        'flutter-tizen build tpk --device-profile wearable -v',
        shell=True,
        cwd=example_dir)
    if completed_process.returncode == 0:
        return True
    else:
        return False


def run_build_examples(args):
    packages_dir = command_utils.get_package_dir()
    target_plugins, _ = command_utils.get_target_plugins(
        packages_dir,
        candidates=args.plugins,
        excludes=args.exclude,
        run_on_changed_packages=args.run_on_changed_packages,
        base_sha=args.base_sha)
    results = []
    for plugin in target_plugins:
        result = _build_examples(os.path.join(packages_dir, plugin))
        results.append(result)

    if False not in results:
        exit(0)
    else:
        exit(1)
