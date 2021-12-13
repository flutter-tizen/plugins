import unittest
import os
import shutil
import subprocess

from commands import command_utils

class TestGetTargetPlugins(unittest.TestCase):

    def setUp(self):
        self.plugins_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), 'plugins'))
        self.packages_dir = os.path.join(self.plugins_dir, 'packages')
        self.existing_packages = {'a', 'b', 'c', 'd', 'e'}
        for package in self.existing_packages:
            os.makedirs(os.path.join(self.packages_dir, package))

    def set_up_git(self):
        for package in self.existing_packages:
            file_path = os.path.join(self.packages_dir, package, 'README.md')
            open(file_path, 'w').close()
        subprocess.run(['git', '-C', self.plugins_dir, 'init', '-q'])
        subprocess.run(['git', '-C', self.plugins_dir, 'add', self.plugins_dir])
        subprocess.run(['git', '-C', self.plugins_dir, 'commit', '-q', '-m', '"Initial commit"'])
        self.changed_packages = {'a', 'b', 'c'}
        for package in self.changed_packages:
            file_path = os.path.join(self.packages_dir, package, 'README.md')
            os.remove(file_path)
        subprocess.run(['git', '-C', self.plugins_dir, 'add', self.plugins_dir])
        subprocess.run(['git', '-C', self.plugins_dir, 'commit', '-q', '-m', '"Change a, b, c"'])
        completed_process = subprocess.run('git rev-parse HEAD^',
                                    shell=True,
                                    cwd=self.plugins_dir,
                                    universal_newlines=True,
                                    stderr=subprocess.PIPE,
                                    stdout=subprocess.PIPE)
        self.base_sha = completed_process.stdout.strip('\n')

    def tearDown(self) -> None:
        shutil.rmtree(self.plugins_dir)

    def test_passing_empty_list_returns_all_existing_plugins(self):
        plugins = []
        testing_plugins, excluded_plugins = command_utils.get_target_plugins(self.packages_dir, candidates=plugins)
        self.assertEqual(set(testing_plugins), self.existing_packages)
        self.assertTrue(len(excluded_plugins) == 0)

    def test_ignores_not_existing_plugins(self):
        plugins = ['a', 'b', 'f']
        testing_plugins, excluded_plugins = command_utils.get_target_plugins(self.packages_dir, candidates=plugins)
        self.assertEqual(set(testing_plugins), {'a', 'b'})
        self.assertTrue(len(excluded_plugins) == 0)

    def test_excludes_plugins(self):
        plugins = []
        excludes = ['a', 'c', 'e']
        testing_plugins, excluded_plugins = command_utils.get_target_plugins(self.packages_dir, candidates=plugins, excludes=excludes)
        self.assertEqual(set(testing_plugins), {'b', 'd'})
        self.assertEqual(set(excluded_plugins), {'a', 'c', 'e'})

    def test_ignore_excludes_that_are_not_test_candidates(self):
        plugins = ['a', 'b', 'c']
        excludes = ['c', 'e']
        testing_plugins, excluded_plugins = command_utils.get_target_plugins(self.packages_dir, candidates=plugins, excludes=excludes)
        self.assertEqual(set(testing_plugins), {'a', 'b'})
        self.assertEqual(set(excluded_plugins), {'c'})

    def test_get_changed_plugins(self):
        self.set_up_git()
        plugins = []
        testing_plugins, excluded_plugins = command_utils.get_target_plugins(self.packages_dir, candidates=plugins, run_on_changed_packages=True, base_sha=self.base_sha)
        self.assertEqual(set(testing_plugins), self.changed_packages)
        self.assertTrue(len(excluded_plugins) == 0)

    def test_ignore_changed_plugins_not_in_candidate(self):
        self.set_up_git()
        plugins = ['a', 'b']
        testing_plugins, excluded_plugins = command_utils.get_target_plugins(self.packages_dir, candidates=plugins, run_on_changed_packages=True, base_sha=self.base_sha)
        self.assertEqual(set(testing_plugins), {'a', 'b'})
        self.assertTrue(len(excluded_plugins) == 0)

    def test_exclude_changed_plugins(self):
        self.set_up_git()
        plugins = []
        excludes = ['a', 'c']
        testing_plugins, excluded_plugins = command_utils.get_target_plugins(self.packages_dir, candidates=plugins, excludes=excludes, run_on_changed_packages=True, base_sha=self.base_sha)
        self.assertEqual(set(testing_plugins), {'b'})
        self.assertEqual(set(excluded_plugins), {'a', 'c'})

if __name__ == '__main__':
    unittest.main()
