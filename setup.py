import os
import subprocess
import sys

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

# LLVM_INSTALL_PATH environment variable must be set before building pytilt
LLVM_INSTALL_PATH = os.getenv('LLVM_INSTALL_PATH')
if(LLVM_INSTALL_PATH == None):
    sys.exit("Error: LLVM_INSTALL_PATH environment variable not defined.")
LLVM_DIR = os.getenv('LLVM_INSTALL_PATH') + '/lib/cmake/llvm'

# extend the Extension class to support CMake build
class CMakeExtension(Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])

class CMakeBuildExtension(build_ext):

    def build_extensions(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError('Cannot find CMake executable')

        for ext in self.extensions:
            self.build_cmake_ext(ext)

    def build_cmake_ext(self, ext):
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        cmake_args = [
            '-DLLVM_DIR={}'.format(LLVM_DIR),
            '-DBUILD_PYTILT=ON',
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={}'.format(extdir),
            '-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={}'.format(self.build_temp),
            '-DPYTHON_EXECUTABLE={}'.format(sys.executable)
        ]

        subprocess.check_call(['cmake'] + cmake_args + ["../.."],
                              cwd = self.build_temp)
        subprocess.check_call(['cmake', '--build', '.'],
                              cwd = self.build_temp)

# setup extension module
setup(
    name = "pytilt",
    version = "0.1",
    ext_modules = [ CMakeExtension( "pytilt" ) ],
    cmdclass={
        'build_ext': CMakeBuildExtension
    },
    zip_safe = False
)