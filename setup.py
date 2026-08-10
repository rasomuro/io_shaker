"""Build the io_shaker Python module and its native extension."""

from pathlib import Path

from setuptools import Extension, setup


ROOT = Path(__file__).parent

sources = [
    "io_shaker_wrap.cxx",
    "io_shaker.cpp",
    "function/fpython.cpp",
    *sorted(
        path.relative_to(ROOT).as_posix()
        for path in (ROOT / "libRSO").glob("*.cpp")
    ),
]

io_shaker_extension = Extension(
    name="_io_shaker",
    sources=sources,
    include_dirs=[".", "function", "libRSO"],
    language="c++",
)

setup(
    name="io_shaker",
    version="0.2.1",
    description="Python bindings for the Reactive Affine Shaker and Inertial Shaker libraries",
    long_description=(ROOT / "README.md").read_text(encoding="utf-8"),
    long_description_content_type="text/markdown",
    py_modules=["io_shaker"],
    ext_modules=[io_shaker_extension],
    python_requires=">=3.8",
    zip_safe=False,
)
