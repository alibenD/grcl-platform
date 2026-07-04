#!/usr/bin/env python3

"""Local SDK boundary drift checker for the G6 wrapper skeletons."""

from __future__ import annotations

from dataclasses import dataclass
import os
import re
import sys
from pathlib import Path


@dataclass(frozen=True)
class Finding:
    path: Path
    line: int
    rule: str
    message: str


FORBIDDEN_PATTERNS: tuple[tuple[str, str, str], ...] = (
    (r"\brclcpp\b", "ros2-sdk-symbol", "references rclcpp, which is outside the wrapper boundary"),
    (r"\brmw\b", "ros2-sdk-symbol", "references rmw, which is outside the wrapper boundary"),
    (
        r"\brcl\b",
        "ros2-sdk-symbol",
        "references rcl, which is outside the wrapper boundary",
    ),
    (
        r"src/grcl-runtime-native",
        "runtime-internal-path",
        "references the runtime-native implementation path",
    ),
    (r"src/grcl-c/src", "runtime-internal-path", "references the grcl-c implementation source path"),
    (r"\bgrcl_runtime_impl\b", "runtime-internal-symbol", "references runtime implementation internals"),
    (r"\bgrcl_backend_[A-Za-z0-9_]*\b", "backend-internal-symbol", "references backend-private symbols"),
    (r"\bnull_backend\b", "backend-internal-symbol", "references backend-private symbols"),
    (
        r"\bpthread_[A-Za-z0-9_]*\b",
        "pthread-api",
        "references pthread APIs, which are outside the wrapper boundary",
    ),
    (r"<pthread\.h>", "pthread-api", "includes pthread APIs, which are outside the wrapper boundary"),
    (r"\bstd::thread\b", "threading-api", "references C++ threading APIs, which are outside the wrapper boundary"),
    (r"\bstd::mutex\b", "threading-api", "references C++ threading APIs, which are outside the wrapper boundary"),
    (
        r"\bstd::condition_variable\b",
        "threading-api",
        "references C++ threading APIs, which are outside the wrapper boundary",
    ),
    (r"<sys/socket\.h>", "socket-api", "references socket APIs, which are outside the wrapper boundary"),
    (r"<arpa/inet\.h>", "socket-api", "references socket APIs, which are outside the wrapper boundary"),
    (r"<netinet/in\.h>", "socket-api", "references socket APIs, which are outside the wrapper boundary"),
    (r"\bsocket\s*\(", "socket-api", "references socket APIs, which are outside the wrapper boundary"),
    (r"\bDockerfile\b", "docker-ci-build", "references Docker, which is outside the wrapper boundary"),
    (r"\bdocker(?:-compose)?\b", "docker-ci-build", "references Docker, which is outside the wrapper boundary"),
    (r"\bCI\b", "docker-ci-build", "references CI, which is outside the wrapper boundary"),
    (r"\.github/workflows", "docker-ci-build", "references CI workflows, which are outside the wrapper boundary"),
    (r"\bCMakeLists\.txt\b", "build-system", "references repo-wide build-system declarations"),
    (r"\bcmake\b", "build-system", "references repo-wide build-system declarations"),
    (r"\bcolcon\b", "build-system", "references repo-wide build-system declarations"),
    (r"\bament\b", "build-system", "references repo-wide build-system declarations"),
    (r"\bcatkin\b", "build-system", "references repo-wide build-system declarations"),
    (r"\bpyproject\.toml\b", "build-system", "references package/build-system declarations"),
    (r"\bsetup\.py\b", "build-system", "references package/build-system declarations"),
    (r"\brequirements\.txt\b", "build-system", "references package/build-system declarations"),
    (r"\bpackage\.xml\b", "build-system", "references package/build-system declarations"),
    (r"\bsetuptools\b", "build-system", "references package/build-system declarations"),
    (r"\bscikit-build\b", "build-system", "references package/build-system declarations"),
    (r"\bpoetry\b", "build-system", "references package/build-system declarations"),
    (r"\bpip\b", "build-system", "references package/build-system declarations"),
    (r"\buv\b", "build-system", "references package/build-system declarations"),
    (r"\bvenv\b", "build-system", "references package/build-system declarations"),
    (r"\bvirtualenv\b", "build-system", "references package/build-system declarations"),
)

PY_IMPORT_FORBIDDEN = re.compile(
    r"^\s*(?:from|import)\s+.*(?:grcl_cpp|grcl\.cpp|grcl-cpp)\b",
    re.IGNORECASE,
)

CPP_PUBLIC_HEADER_INCLUDE = re.compile(r"^\s*#include\s+<grcl/c/[^>]+>\s*$")
CPP_ABI_COPY_DECLARATION = re.compile(r"\b(?:typedef\s+struct\s+grcl_|struct\s+grcl_[A-Za-z0-9_]*\s*\{)")


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def workspace_root(repo_root_path: Path) -> Path | None:
    parent = repo_root_path.parent
    if parent.name == "src":
        return parent.parent
    return None


def artifact_root(repo_root_path: Path) -> Path:
    env_value = os.environ.get("GRCL_PLATFORM_ARTIFACT_ROOT")
    if env_value:
        return Path(env_value)

    workspace = workspace_root(repo_root_path)
    if workspace is None:
        raise RuntimeError(
            "FAIL artifact root resolution (set GRCL_PLATFORM_ARTIFACT_ROOT when the workspace "
            "layout is not <workspace>/src/grcl-platform)"
        )
    return workspace / "artifacts"


def iter_scan_files(repo_root_path: Path) -> list[Path]:
    files: list[Path] = []
    cpp_root = repo_root_path / "src" / "grcl-cpp"
    py_root = repo_root_path / "src" / "grcl-py"

    cpp_patterns = [
        cpp_root / "README.md",
        cpp_root / "include" / "grcl" / "cpp" / "*.hpp",
    ]
    py_patterns = [
        py_root / "README.md",
        py_root / "grcl_py" / "*.py",
    ]

    for pattern in cpp_patterns + py_patterns:
        files.extend(sorted(pattern.parent.glob(pattern.name)) if pattern.name != "*.py" and pattern.name != "*.hpp" else sorted(pattern.parent.glob(pattern.name)))

    files = [path for path in files if path.is_file()]
    return sorted(files)


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def line_iter(text: str) -> list[str]:
    return text.splitlines()


def scan_generic_patterns(path: Path, text: str) -> list[Finding]:
    findings: list[Finding] = []
    for line_number, line in enumerate(line_iter(text), start=1):
        for pattern, rule, message in FORBIDDEN_PATTERNS:
            if re.search(pattern, line, re.IGNORECASE):
                findings.append(Finding(path=path, line=line_number, rule=rule, message=message))
    return findings


def scan_cpp_header(path: Path, text: str) -> list[Finding]:
    findings: list[Finding] = []
    if not path.match("src/grcl-cpp/include/grcl/cpp/*.hpp"):
        return findings

    lines = line_iter(text)
    has_public_include = any(CPP_PUBLIC_HEADER_INCLUDE.match(line) for line in lines)
    if not has_public_include:
        findings.append(
            Finding(
                path=path,
                line=1,
                rule="cpp-public-header-include",
                message="does not include a public grcl/c/*.h header",
            )
        )

    for line_number, line in enumerate(lines, start=1):
        if CPP_ABI_COPY_DECLARATION.search(line):
            findings.append(
                Finding(
                    path=path,
                    line=line_number,
                    rule="cpp-abi-copy",
                    message="appears to copy ABI declarations instead of wrapping the public header",
                )
            )
    return findings


def scan_py_private_boundary(path: Path, text: str) -> list[Finding]:
    findings: list[Finding] = []
    if not path.match("src/grcl-py/grcl_py/*.py"):
        return findings

    for line_number, line in enumerate(line_iter(text), start=1):
        if PY_IMPORT_FORBIDDEN.search(line):
            findings.append(
                Finding(
                    path=path,
                    line=line_number,
                    rule="py-import-source",
                    message="imports grcl-cpp instead of routing through the private native boundary",
                )
            )
    return findings


def scan_path(path: Path) -> list[Finding]:
    text = read_text(path)
    findings = scan_generic_patterns(path, text)
    findings.extend(scan_cpp_header(path, text))
    findings.extend(scan_py_private_boundary(path, text))
    return findings


def write_report(report_path: Path, repo_root_path: Path, artifact_root_path: Path, findings: list[Finding], scanned_files: list[Path]) -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    status = "FAIL" if findings else "PASS"
    lines = [
        f"sdk boundary drift check: {status.lower()}",
        f"repository_root: {repo_root_path}",
        f"artifact_root: {artifact_root_path}",
        f"scanned_files: {len(scanned_files)}",
        "scanned_paths:",
    ]
    for path in scanned_files:
        lines.append(f"- {path.relative_to(repo_root_path)}")
    lines.append(f"violations: {len(findings)}")
    for finding in findings:
        rel_path = finding.path.relative_to(repo_root_path)
        lines.append(f"- {rel_path}:{finding.line} [{finding.rule}] {finding.message}")
    report_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    repo_root_path = repo_root()
    artifact_root_path = artifact_root(repo_root_path)
    report_path = artifact_root_path / "g6" / "sdk-boundaries" / "sdk-boundary-drift-report.txt"

    scanned_files = iter_scan_files(repo_root_path)
    findings: list[Finding] = []
    for path in scanned_files:
        findings.extend(scan_path(path))

    write_report(report_path, repo_root_path, artifact_root_path, findings, scanned_files)

    if findings:
        print(f"FAIL sdk boundary drift check ({report_path})", file=sys.stderr)
        for finding in findings:
            rel_path = finding.path.relative_to(repo_root_path)
            print(
                f"- {rel_path}:{finding.line} [{finding.rule}] {finding.message}",
                file=sys.stderr,
            )
        return 1

    print(f"PASS sdk boundary drift check ({report_path})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
