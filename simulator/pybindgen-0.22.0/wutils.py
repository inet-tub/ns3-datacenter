import os.path
import sys
import subprocess
import re


def get_version(path=None):
    filename = os.path.join(os.path.dirname(__file__), 'pybindgen', 'version.py')
    if os.path.exists(filename):
        # Read the version.py from the version file
        with open(filename, "rt") as versionpy:
            for line in versionpy:
                try:
                    head, rest = line.split("version = ")
                except ValueError:
                    continue
                return eval(rest)
            return version_str
    return 'unknown'
