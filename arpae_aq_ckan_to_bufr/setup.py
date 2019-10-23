from setuptools import setup


setup(
    name='arpae_aq_ckan_to_bufr',
    version='0.1.1',
    description='Python Distribution Utilities',
    author='Emanuele Di Giacomo',
    author_email='emanuele@digiacomo.cc',
    url='https://github.com/r-map/rmap',
    packages=['arpae_aq_ckan_to_bufr'],
    license="GNU GPL v2",
    entry_points={
        'console_scripts': [
            'arpae_aq_ckan_to_bufr = arpae_aq_ckan_to_bufr:main'
        ],
    },
)
