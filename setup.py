"""Build the pylion Python module and its native extension."""

from pathlib import Path

from setuptools import Extension, setup


ROOT = Path(__file__).parent

sources = [
    "pylion_wrap.cxx",
    "pylion.cpp",
    "function/fpython.cpp",
    *sorted(
        path.relative_to(ROOT).as_posix()
        for path in (ROOT / "libRSO").glob("*.cpp")
    ),
]

pylion_extension = Extension(
    name="_pylion",
    sources=sources,
    include_dirs=[".", "function", "libRSO"],
    language="c++",
)

setup(
    name="pylion",
    version="0.1.2",
    description="Python bindings for the Reactive Search Optimization library",
    long_description=(ROOT / "README.md").read_text(encoding="utf-8"),
    long_description_content_type="text/markdown",
    py_modules=["pylion"],
    ext_modules=[pylion_extension],
    python_requires=">=3.8",
    zip_safe=False,
)
