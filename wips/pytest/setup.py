from setuptools import setup, Extension
import glob
import sysconfig

sources = glob.glob('native/*.c')
python_include_dir = sysconfig.get_paths()["include"]

wips_extension = Extension(
    'wips',
    sources=sources,
    define_macros=[("WIPS_OPTION_BUILD_PYTHON", None)],
    extra_compile_args=["-O3","-Wall", "-Wextra"],
    include_dirs=[python_include_dir]
)

setup(
    name="wips",
    version="0.1",
    ext_modules=[wips_extension],
)

# python3 setup.py build_ext --inplace