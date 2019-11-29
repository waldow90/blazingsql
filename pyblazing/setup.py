from setuptools import setup, find_packages

setup(
    name='blazingsql',
    version="0.6",
    description="BlazingSQL engine",
    url="https://github.com/BlazingDB/pyBlazing/",
    author="BlazingSQL",
    license="Apache 2.0",
    classifiers=[
        "Intended Audience :: Developers",
        "Topic :: Database",
        "Topic :: Scientific/Engineering",
        "License :: OSI Approved :: Apache Software License",
        "Programming Language :: Python",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
    ],
    packages=find_packages(include=["pyblazing", "pyblazing.*", "blazingsql", "blazingsql.*"]),
    zip_safe=False,
)
