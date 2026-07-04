#!/usr/bin/env python3
"""Run GRCL C ABI and public-header conformance checks."""

from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve()
REPO_ROOT = SCRIPT_PATH.parent.parent
WORKSPACE_ROOT = REPO_ROOT.parent.parent
DEFAULT_ARTIFACT_ROOT = WORKSPACE_ROOT / "artifacts"
INCLUDE_DIR = REPO_ROOT / "src" / "grcl-c" / "include"
PUBLIC_HEADER_DIR = INCLUDE_DIR / "grcl" / "c"
C_SMOKE_SOURCE = REPO_ROOT / "src" / "grcl-c" / "tests" / "compile_headers_smoke.c"
CPP_SMOKE_SOURCE = REPO_ROOT / "src" / "grcl-c" / "tests" / "compile_headers_smoke.cpp"


@dataclass(frozen=True)
class CompileSpec:
    label: str
    compiler_env: str
    default_compiler: str
    standard: str
    source: Path
    output_name: str


@dataclass(frozen=True)
class ForbiddenPattern:
    label: str
    pattern: re.Pattern[str]


COMPILE_SPECS = (
    CompileSpec(
        label="c11 header smoke",
        compiler_env="CC",
        default_compiler="cc",
        standard="-std=c11",
        source=C_SMOKE_SOURCE,
        output_name="compile_headers_smoke.c.o",
    ),
    CompileSpec(
        label="c++17 header smoke",
        compiler_env="CXX",
        default_compiler="c++",
        standard="-std=c++17",
        source=CPP_SMOKE_SOURCE,
        output_name="compile_headers_smoke.cpp.o",
    ),
)

FORBIDDEN_PATTERNS = (
    ForbiddenPattern("C++ standard library namespace", re.compile(r"\bstd::")),
    ForbiddenPattern("C++ standard library include", re.compile(r"#\s*include\s*<\s*(string|vector|map|unordered_map|memory|optional|variant|exception|stdexcept|typeinfo)\s*>")),
    ForbiddenPattern("C++ template declaration", re.compile(r"\btemplate\s*<")),
    ForbiddenPattern("C++ exception keyword", re.compile(r"\b(throw|try|catch)\b")),
    ForbiddenPattern("C++ RTTI use", re.compile(r"\b(dynamic_cast|typeid)\b")),
    ForbiddenPattern("ROS rcl identifier", re.compile(r"\brcl_[A-Za-z0-9_]*\b|\brcl\b")),
    ForbiddenPattern("ROS rmw identifier", re.compile(r"\brmw_[A-Za-z0-9_]*\b|\brmw\b")),
    ForbiddenPattern("ROS rclcpp identifier", re.compile(r"\brclcpp\b")),
    ForbiddenPattern("raw ROS identifier", re.compile(r"\bROS2_[A-Za-z0-9_]*\b|\bROS_[A-Za-z0-9_]*\b|\bros2_[A-Za-z0-9_]*\b|\bROSIDL_[A-Za-z0-9_]*\b|\brosidl_[A-Za-z0-9_]*\b|\brcutils_[A-Za-z0-9_]*\b")),
    ForbiddenPattern("socket API", re.compile(r"\b(socket|sockaddr|socklen_t|send|recv|bind|listen|accept|connect)\s*\(|\bsockaddr(_in|_in6|_storage)?\b|\bsocklen_t\b")),
    ForbiddenPattern("pthread API", re.compile(r"\bpthread_[A-Za-z0-9_]*\b")),
    ForbiddenPattern("Docker declaration", re.compile(r"\b(Dockerfile|docker\s+build|docker\s+run|FROM\s+[A-Za-z0-9_./:-]+)\b")),
    ForbiddenPattern("CI declaration", re.compile(r"\b(GitHub Actions|\.github/workflows|continuous integration|CI)\b")),
    ForbiddenPattern("CMake declaration", re.compile(r"\b(cmake_minimum_required|add_executable|add_library|target_link_libraries|target_include_directories|project\s*\()\b")),
    ForbiddenPattern("ROS/build-system declaration", re.compile(r"\b(colcon|ament_[A-Za-z0-9_]*|catkin_[A-Za-z0-9_]*)\b")),
)

REQUIRED_ABI_STRUCTS = (
    "grcl_runtime_options",
    "grcl_allocator_config",
    "grcl_backend_runtime_context",
    "grcl_backend_descriptor",
    "grcl_backend_ops",
)


def artifact_root() -> Path:
    configured = os.environ.get("GRCL_PLATFORM_ARTIFACT_ROOT")
    return Path(configured) if configured else DEFAULT_ARTIFACT_ROOT


def public_headers() -> list[Path]:
    headers = sorted(PUBLIC_HEADER_DIR.glob("*.h"))
    if not headers:
        raise FileNotFoundError(f"no public headers found under {PUBLIC_HEADER_DIR}")
    return headers


def require_files(paths: tuple[Path, ...]) -> list[str]:
    failures = []
    for path in paths:
        if not path.is_file():
            failures.append(f"required file is missing: {path}")
    return failures


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*", "", text)


def display_header_path(header: Path) -> Path:
    try:
        return header.relative_to(REPO_ROOT)
    except ValueError:
        return header


def scan_forbidden_header_content(headers: list[Path]) -> list[str]:
    failures = []
    for header in headers:
        text = strip_comments(header.read_text(encoding="utf-8"))
        for line_no, line in enumerate(text.splitlines(), start=1):
            for forbidden in FORBIDDEN_PATTERNS:
                if forbidden.pattern.search(line):
                    failures.append(f"{display_header_path(header)}:{line_no}: forbidden {forbidden.label}: {line.strip()}")
    return failures


def combined_header_text(headers: list[Path]) -> str:
    return "\n".join(header.read_text(encoding="utf-8") for header in headers)


def find_struct_body(text: str, struct_name: str) -> str | None:
    pattern = re.compile(
        rf"(?:typedef\s+)?struct\s+{re.escape(struct_name)}\s*\{{(?P<body>.*?)\}}\s*(?:[A-Za-z_][A-Za-z0-9_]*\s*)?;",
        re.DOTALL,
    )
    match = pattern.search(text)
    if match is None:
        return None
    return match.group("body")


def check_required_abi_structs(headers: list[Path]) -> list[str]:
    text = strip_comments(combined_header_text(headers))
    failures = []
    for struct_name in REQUIRED_ABI_STRUCTS:
        body = find_struct_body(text, struct_name)
        if body is None:
            failures.append(f"struct {struct_name}: required ABI struct is missing")
            continue
        struct_size = re.search(r"\bsize_t\s+struct_size\s*;", body)
        abi_version = re.search(r"\buint32_t\s+abi_version\s*;", body)
        if struct_size is None:
            failures.append(f"struct {struct_name}: missing size_t struct_size field")
        if abi_version is None:
            failures.append(f"struct {struct_name}: missing uint32_t abi_version field")
        if struct_size is not None and abi_version is not None and struct_size.start() > abi_version.start():
            failures.append(f"struct {struct_name}: struct_size must appear before abi_version")
    return failures


def compiler_command(spec: CompileSpec, output_root: Path) -> list[str]:
    compiler = os.environ.get(spec.compiler_env, spec.default_compiler)
    return [
        compiler,
        spec.standard,
        "-I",
        str(INCLUDE_DIR),
        "-c",
        str(spec.source),
        "-o",
        str(output_root / spec.output_name),
    ]


def run_compile(spec: CompileSpec, output_root: Path) -> tuple[bool, str]:
    compiler = os.environ.get(spec.compiler_env, spec.default_compiler)
    if shutil.which(compiler) is None:
        return False, f"{spec.label}: compiler not found: {compiler}"

    command = compiler_command(spec, output_root)
    result = subprocess.run(command, check=False, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stem = spec.output_name.replace(".", "_")
    (output_root / f"{stem}.stdout.txt").write_text(result.stdout, encoding="utf-8")
    (output_root / f"{stem}.stderr.txt").write_text(result.stderr, encoding="utf-8")
    (output_root / f"{stem}.command.txt").write_text(" ".join(command) + "\n", encoding="utf-8")
    if result.returncode != 0:
        return False, f"{spec.label}: compiler exited {result.returncode}; stderr: {result.stderr.strip()}"
    return True, f"{spec.label}: ok"


def write_report(output_root: Path, header_count: int, compile_messages: list[str]) -> Path:
    report_path = output_root / "c-abi-conformance-report.txt"
    content = [
        "c abi conformance: ok",
        f"repository_root: {REPO_ROOT}",
        f"public_header_count: {header_count}",
        "compile_checks:",
        *[f"- {message}" for message in compile_messages],
        "abi_struct_checks:",
        *[f"- {struct_name}: struct_size and abi_version present" for struct_name in REQUIRED_ABI_STRUCTS],
    ]
    report_path.write_text("\n".join(content) + "\n", encoding="utf-8")
    return report_path


def main() -> int:
    output_root = artifact_root() / "g5" / "c-abi"
    output_root.mkdir(parents=True, exist_ok=True)

    headers = public_headers()
    failures = require_files((C_SMOKE_SOURCE, CPP_SMOKE_SOURCE))
    failures.extend(scan_forbidden_header_content(headers))
    failures.extend(check_required_abi_structs(headers))

    if failures:
        for failure in failures:
            print(f"error: {failure}", file=sys.stderr)
        return 1

    compile_messages = []
    for spec in COMPILE_SPECS:
        ok, message = run_compile(spec, output_root)
        compile_messages.append(message)
        if not ok:
            print(f"error: {message}", file=sys.stderr)
            return 1

    report_path = write_report(output_root, len(headers), compile_messages)
    print(f"c abi checks passed: {len(headers)} public headers")
    for message in compile_messages:
        print(message)
    print(f"report: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
