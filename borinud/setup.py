import os
from setuptools import setup, find_packages


def read(*rnames):
    return open(os.path.join(os.path.dirname(__file__), *rnames)).read()

name = "borinud"
version = "0.7"
release = version + ".4"

setup(
    name=name,
    version=release,
    author="Emanuele Di Giacomo",
    author_email="edigiacomo@arpa.emr.it",
    description="weather data web api",
    long_description="\n".join((
        read("README.rst"),
        read("CHANGES.rst"),
        read("CONTRIBUTORS.rst"),
    )),
    license="GPLv2+",
    keywords="ARPA SIMC weather station sensor GIS",
    url="http://sourceforge.net/projects/r-map/",
    packages=find_packages(),
    namespace_packages=[],
    classifiers=[
    ],
    test_suite="borinud",
    install_requires=[
        'bottle',
    ],
    extras_require={
    },
    command_options={
        'build_sphinx': {
            'project': ('setup.py', name),
            'version': ('setup.py', version),
            'release': ('setup.py', release),
            'source_dir': ('setup.py', 'doc/source'),
            'build_dir': ('setup.py', 'doc/build'),
        }
    },
    scripts=[
    ],
    include_package_data=True,
)
