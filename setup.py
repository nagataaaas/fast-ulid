from setuptools import setup, Extension

fast_ulid__title__ = "fast-ulid"
fast_ulid__description__ = "Easy to use ULID generator"
fast_ulid__url__ = "https://github.com/nagataaaas/fast-ulid"
fast_ulid__version_info__ = ("0", "1", "3")
fast_ulid__version__ = ".".join(fast_ulid__version_info__)
fast_ulid__author__ = "Yamato Nagata"
fast_ulid__author_email__ = "chickenwingswillfly@gmail.com"
fast_ulid__maintainer__ = "Yamato Nagata"
fast_ulid__license__ = "MIT"
fast_ulid__copyright__ = "(c) 2022 by Yamato Nagata"

setup(name=fast_ulid__title__,
      version=fast_ulid__version__,
      url=fast_ulid__url__,
      license=fast_ulid__license__,
      author=fast_ulid__author__,
      author_email=fast_ulid__author_email__,
      description=fast_ulid__description__,
      long_description=__doc__,
      long_description_content_type="text/markdown",
      platforms="any",
      classifiers=[
          "Development Status :: 4 - Beta",
          "Environment :: Other Environment",
          "Intended Audience :: Developers",
          "License :: OSI Approved :: MIT License",
          "Operating System :: OS Independent",
          "Programming Language :: Python :: 3",
          "Programming Language :: Python",
          "Topic :: Software Development :: Libraries :: Python Modules"
      ],
      packages=["fast_ulid"],
      package_data={
          'fast_ulid': ["py.typed", "*.pyi"]
      },
      ext_modules=[
          Extension('fast_ulid', ['./fast_ulid/ULID.cpp'], language='c++'),
      ])
