#!/usr/bin/env python3
"""Run GRCL documentation conformance checks."""

from __future__ import annotations

import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from urllib.parse import unquote, urlparse


SCRIPT_PATH = Path(__file__).resolve()
REPO_ROOT = SCRIPT_PATH.parent.parent
WORKSPACE_ROOT = REPO_ROOT.parent.parent
DEFAULT_ARTIFACT_ROOT = WORKSPACE_ROOT / "artifacts"
DOC_GLOB_ROOTS = (
    REPO_ROOT / "README.md",
    REPO_ROOT / "AGENTS.md",
)
STATUS_PATHS = (
    REPO_ROOT / "docs" / "status" / "current-context.md",
    REPO_ROOT / "docs" / "status" / "goal-execution-queue.md",
    REPO_ROOT / "docs" / "status" / "middleware-goal-roadmap.md",
)
LINK_PATTERN = re.compile(r"!?(\[[^\]]+\])\(([^)\s]+(?: [^)]+)?)\)")
INLINE_CODE_PATTERN = re.compile(r"`[^`\n]+`")
FENCED_CODE_PATTERN = re.compile(r"```.*?```", re.DOTALL)
UNFINISHED_PATTERNS = (
    re.compile(r"\bTBD\b", re.IGNORECASE),
    re.compile(r"\bTODO\b", re.IGNORECASE),
    re.compile(r"\bfill in\b", re.IGNORECASE),
    re.compile(r"\bimplement later\b", re.IGNORECASE),
    re.compile(r"\bplace holder\b", re.IGNORECASE),
)


@dataclass(frozen=True)
class StatusExpectation:
    label: str
    patterns: tuple[re.Pattern[str], ...]


STATUS_EXPECTATIONS_BY_FILE = {
    "current-context.md": (
        StatusExpectation(
            label="M1 complete",
            patterns=(
                re.compile(r"\bM1\b.*\bcomplete\b", re.IGNORECASE),
                re.compile(r"\bFirst Runnable GRCL-C Core\b.*\bcomplete\b", re.IGNORECASE),
            ),
        ),
        StatusExpectation(
            label="G5 complete",
            patterns=(
                re.compile(r"\bG5\b.*\bplanning\b.*\bcomplete\b", re.IGNORECASE),
                re.compile(r"\bG5\b.*\bimplementation\b.*\bcomplete\b", re.IGNORECASE),
            ),
        ),
        StatusExpectation(
            label="G6 planning complete",
            patterns=(
                re.compile(r"\bG6\b.*\bplanning\b.*\bcomplete\b", re.IGNORECASE),
                re.compile(r"\bG6 planning status:\s*`?complete`?", re.IGNORECASE),
            ),
        ),
        StatusExpectation(
            label="G6 option A selected",
            patterns=(
                re.compile(r"\bOption A\b", re.IGNORECASE),
                re.compile(r"\bctypes\b.*\bprivate dynamic-library shim\b", re.IGNORECASE),
            ),
        ),
        StatusExpectation(
            label="G6 implementation authorized or active",
            patterns=(
                re.compile(r"\bG6\b.*\bimplementation\b.*\bauthorized\b", re.IGNORECASE),
                re.compile(r"\bG6\b.*\bimplementation\b.*\bactive\b", re.IGNORECASE),
                re.compile(r"\bG6\b.*\bimplementation\b.*\bcomplete\b", re.IGNORECASE),
                re.compile(r"\bG6 implementation status:\s*`?(?:authorized|active|complete)`?", re.IGNORECASE),
            ),
        ),
    ),
    "goal-execution-queue.md": (
        StatusExpectation(
            label="G6 implementation authorized or active",
            patterns=(
                re.compile(r"\bG6\b.*\bimplementation\b.*\bauthorized\b", re.IGNORECASE),
                re.compile(r"\bG6\b.*\bimplementation\b.*\bactive\b", re.IGNORECASE),
                re.compile(r"\bG6\b.*\bimplementation\b.*\bauthorized\b.*\bactive\b", re.IGNORECASE),
            ),
        ),
    ),
    "middleware-goal-roadmap.md": (
        StatusExpectation(
            label="M1 complete",
            patterns=(
                re.compile(r"\bM1\b.*\bcomplete\b", re.IGNORECASE),
                re.compile(r"\bFirst Runnable GRCL-C Core\b.*\bcomplete\b", re.IGNORECASE),
            ),
        ),
        StatusExpectation(
            label="G5 complete",
            patterns=(
                re.compile(r"\bG5\b.*\bplanning\b.*\bcomplete\b", re.IGNORECASE),
                re.compile(r"\bG5\b.*\bimplementation\b.*\bcomplete\b", re.IGNORECASE),
            ),
        ),
        StatusExpectation(
            label="G6 planning complete",
            patterns=(
                re.compile(r"\bG6\b.*\bplanning\b.*\bcomplete\b", re.IGNORECASE),
                re.compile(r"\bM2\b.*\bCross-Language SDK Boundary Baseline\b", re.IGNORECASE),
            ),
        ),
        StatusExpectation(
            label="G6 option A selected",
            patterns=(
                re.compile(r"\bOption A\b", re.IGNORECASE),
                re.compile(r"\bctypes\b.*\bprivate dynamic-library shim\b", re.IGNORECASE),
            ),
        ),
    ),
}


def iter_doc_paths() -> list[Path]:
    docs_dir = REPO_ROOT / "docs"
    return [*DOC_GLOB_ROOTS, *sorted(docs_dir.rglob("*.md"))]


def strip_code(text: str) -> str:
    without_fences = FENCED_CODE_PATTERN.sub("", text)
    return INLINE_CODE_PATTERN.sub("", without_fences)


def should_ignore_link(target: str) -> bool:
    if not target or target.startswith("#"):
        return True
    parsed = urlparse(target)
    if parsed.scheme in {"http", "https", "mailto"}:
        return True
    return False


def resolve_local_target(source_path: Path, target: str) -> Path | None:
    clean_target = target.split("#", 1)[0].strip()
    if not clean_target:
        return None
    parsed = urlparse(clean_target)
    if parsed.scheme:
        return None
    decoded_path = unquote(parsed.path)
    if not decoded_path:
        return None
    return (source_path.parent / decoded_path).resolve()


def check_links(doc_paths: list[Path]) -> list[str]:
    failures: list[str] = []
    for doc_path in doc_paths:
        text = doc_path.read_text(encoding="utf-8")
        for match in LINK_PATTERN.finditer(text):
            target = match.group(2).strip()
            if should_ignore_link(target):
                continue
            resolved = resolve_local_target(doc_path, target)
            if resolved is None:
                continue
            if not resolved.exists():
                rel_doc = doc_path.relative_to(REPO_ROOT)
                failures.append(f"{rel_doc}: unresolved link target {target!r}")
    return failures


def check_unfinished_markers(doc_paths: list[Path]) -> list[str]:
    failures: list[str] = []
    for doc_path in doc_paths:
        text = strip_code(doc_path.read_text(encoding="utf-8"))
        for line_no, line in enumerate(text.splitlines(), start=1):
            for pattern in UNFINISHED_PATTERNS:
                if pattern.search(line):
                    rel_doc = doc_path.relative_to(REPO_ROOT)
                    failures.append(f"{rel_doc}:{line_no}: unfinished marker matched {pattern.pattern!r}")
    return failures


def check_status_consistency() -> tuple[list[str], list[str]]:
    failures: list[str] = []
    satisfied_labels: list[str] = []
    for status_path in STATUS_PATHS:
        rel_path = status_path.relative_to(REPO_ROOT)
        expectations = STATUS_EXPECTATIONS_BY_FILE.get(status_path.name, ())
        text = status_path.read_text(encoding="utf-8")
        normalized = " ".join(text.split())
        for expectation in expectations:
            if any(pattern.search(normalized) for pattern in expectation.patterns):
                satisfied_labels.append(f"{rel_path}: {expectation.label}")
                continue
            failures.append(f"{rel_path}: missing status assertion for {expectation.label}")
    return failures, satisfied_labels


def artifact_root() -> Path:
    configured = os.environ.get("GRCL_PLATFORM_ARTIFACT_ROOT")
    return Path(configured) if configured else DEFAULT_ARTIFACT_ROOT


def write_report(doc_paths: list[Path], status_lines: list[str], issue_count: int) -> Path:
    report_dir = artifact_root() / "g5" / "docs"
    report_dir.mkdir(parents=True, exist_ok=True)
    report_path = report_dir / "documentation-conformance-report.txt"
    content = [
        "documentation conformance: ok",
        f"repository_root: {REPO_ROOT}",
        f"document_count: {len(doc_paths)}",
        f"status_assertions_checked: {len(status_lines)}",
        f"issue_count: {issue_count}",
        "status_assertions:",
        *[f"- {line}" for line in status_lines],
    ]
    report_path.write_text("\n".join(content) + "\n", encoding="utf-8")
    return report_path


def main() -> int:
    doc_paths = iter_doc_paths()
    failures = []
    failures.extend(check_links(doc_paths))
    failures.extend(check_unfinished_markers(doc_paths))
    status_failures, satisfied_labels = check_status_consistency()
    failures.extend(status_failures)

    if failures:
        for failure in failures:
            print(f"error: {failure}", file=sys.stderr)
        return 1

    report_path = write_report(doc_paths, satisfied_labels, issue_count=0)
    print(f"documentation checks passed: {len(doc_paths)} files")
    print(f"status assertions checked: {len(satisfied_labels)}")
    print(f"report: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
