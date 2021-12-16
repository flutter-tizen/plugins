import subprocess
import os


def set_parser_arguments(parser,
                         plugins=False,
                         exclude=False,
                         run_on_changed_packages=False,
                         base_sha=False,
                         timeout=False,
                         command=''):
    if plugins:
        parser.add_argument('--plugins',
                            type=str,
                            nargs='*',
                            default=[],
                            help=f'Specifies which plugins to {command}. \
            If it is not specified then it will include every plugin under packages.')

    if exclude:
        parser.add_argument('--exclude',
                            type=str,
                            nargs='*',
                            default=[],
                            help=f'Exclude plugins from {command}.')

    if run_on_changed_packages:
        parser.add_argument('--run-on-changed-packages',
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


def _get_changed_plugins(packages_dir, base_sha=''):
    if base_sha == '':
        base_sha = subprocess.run(
            'git merge-base --fork-point FETCH_HEAD HEAD',
            shell=True,
            cwd=packages_dir,
            encoding='utf-8',
            stdout=subprocess.PIPE).stdout.strip()
        if base_sha == '':
            base_sha = subprocess.run('git merge-base FETCH_HEAD HEAD',
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
        if index < len(path_segments) and path_segments[
                index + 1] not in changed_plugins:
            changed_plugins.append(path_segments[index + 1])

    return list(set(changed_plugins))


def get_target_plugins(packages_dir,
                       candidates=[],
                       excludes=[],
                       run_on_changed_packages=False,
                       base_sha=''):
    # If no candidates are provided, all packages under `/packages` are candidates.
    existing_packages = os.listdir(packages_dir)
    if not candidates:
        candidates = existing_packages[:]

    # Remove non existing package names.
    existing_candidates = []
    for candidate in candidates:
        if candidate not in existing_packages:
            print(f'{candidate} package does not exist, ignoring input...')
        else:
            existing_candidates.append(candidate)
    existing_excludes = []
    for exclude in excludes:
        if exclude not in existing_packages:
            print(f'{exclude} package does not exist, ignoring input...')
        else:
            existing_excludes.append(exclude)

    # If `run_on_changed_packages` is true, get the subset of `existing_candidates` where each package is changed.
    if run_on_changed_packages:
        changed_packages = _get_changed_plugins(packages_dir, base_sha)
        for changed_package in changed_packages:
            if changed_package not in existing_candidates:
                print(f'{changed_package} package is changed but is not tested because it\'t not one of the candidates.')
        existing_candidates = list(set(existing_candidates).intersection(set(changed_packages)))

    # Remove excludes that are not in `existing candidates`.
    final_excludes = []
    for existing_exclude in existing_excludes:
        if existing_exclude not in existing_candidates:
            print(f'{existing_exclude} package does not exist in candidates, ignoring input...')
        else:
            final_excludes.append(existing_exclude)

    # Get the final candidates for testing by removing excludes.
    final_candidates = list(set(existing_candidates).difference(set(final_excludes)))
    return final_candidates, final_excludes


def get_package_dir():
    return os.path.abspath(
        os.path.join(os.path.dirname(__file__), '../../packages'))
