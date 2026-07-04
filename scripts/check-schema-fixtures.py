#!/usr/bin/env python3
"""Validate GRCL schema fixtures using a small YAML subset parser."""

from __future__ import annotations

import argparse
import json
import os
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any


class ParseError(Exception):
    """Raised when a YAML-like document cannot be parsed."""


class ValidationError(Exception):
    """Raised when data does not match the supported schema subset."""


@dataclass(frozen=True)
class RuntimeFixtureSpec:
    filename: str
    expected_status: str


@dataclass(frozen=True)
class FixtureSpec:
    filename: str


SCRIPT_PATH = Path(__file__).resolve()
REPO_ROOT = SCRIPT_PATH.parent.parent
WORKSPACE_ROOT = REPO_ROOT.parent.parent
DEFAULT_ARTIFACT_ROOT = WORKSPACE_ROOT / "artifacts"
RUNTIME_SCHEMA_PATH = REPO_ROOT / "schemas" / "runtime-capability-record.schema.yaml"
NEGOTIATION_SCHEMA_PATH = REPO_ROOT / "schemas" / "capability-negotiation-result.schema.yaml"
MCU_PROFILE_SCHEMA_PATH = REPO_ROOT / "schemas" / "mcu-profile.schema.yaml"
RUNTIME_FIXTURE_SPECS = (
    RuntimeFixtureSpec("accepted.yaml", "accepted"),
    RuntimeFixtureSpec("degraded_accepted.yaml", "degraded_accepted"),
    RuntimeFixtureSpec("rejected_incompatible.yaml", "rejected_incompatible"),
)
MCU_PROFILE_FIXTURE_SPECS = (
    FixtureSpec("baremetal-min.yaml"),
    FixtureSpec("rtos-basic.yaml"),
    FixtureSpec("rtos-posix-lite.yaml"),
)
FORBIDDEN_RECORD_KEYS = {"availability", "health", "resources", "pressure", "rate_limits", "degradation"}


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--runtime-capability",
        action="store_true",
        help="validate runtime capability conformance fixtures",
    )
    group.add_argument(
        "--mcu-profiles",
        action="store_true",
        help="reserved for future MCU profile fixture validation",
    )
    return parser.parse_args(argv)


def load_yaml_like(path: Path) -> Any:
    if not path.is_file():
        raise ValidationError(f"required file is missing: {path}")

    lines = path.read_text(encoding="utf-8").splitlines()
    entries = []
    for line_no, raw_line in enumerate(lines, start=1):
        if not raw_line.strip():
            continue
        indent = len(raw_line) - len(raw_line.lstrip(" "))
        if indent % 2 != 0:
            raise ParseError(f"{path}:{line_no}: indentation must use multiples of 2 spaces")
        entries.append((line_no, indent, raw_line[indent:]))

    if not entries:
        raise ParseError(f"{path}: empty documents are not supported")

    value, next_index = parse_block(path, entries, 0, entries[0][1])
    if next_index != len(entries):
        line_no = entries[next_index][0]
        raise ParseError(f"{path}:{line_no}: trailing content could not be parsed")
    return value


def parse_block(path: Path, entries: list[tuple[int, int, str]], index: int, indent: int) -> tuple[Any, int]:
    if index >= len(entries):
        raise ParseError(f"{path}: unexpected end of document")
    _, line_indent, content = entries[index]
    if line_indent != indent:
        raise ParseError(f"{path}:{entries[index][0]}: expected indentation {indent}, found {line_indent}")
    if content.startswith("- "):
        return parse_sequence(path, entries, index, indent)
    return parse_mapping(path, entries, index, indent)


def parse_mapping(path: Path, entries: list[tuple[int, int, str]], index: int, indent: int) -> tuple[dict[str, Any], int]:
    mapping: dict[str, Any] = {}
    while index < len(entries):
        line_no, line_indent, content = entries[index]
        if line_indent < indent:
            break
        if line_indent > indent:
            raise ParseError(f"{path}:{line_no}: unexpected indentation inside mapping")
        if content.startswith("- "):
            break
        key, value, has_value = split_key_value(path, line_no, content)
        if has_value:
            mapping[key] = parse_scalar(value)
            index += 1
            continue
        index += 1
        if index >= len(entries) or entries[index][1] <= line_indent:
            raise ParseError(f"{path}:{line_no}: expected nested block for key '{key}'")
        nested, index = parse_block(path, entries, index, line_indent + 2)
        mapping[key] = nested
    return mapping, index


def parse_sequence(path: Path, entries: list[tuple[int, int, str]], index: int, indent: int) -> tuple[list[Any], int]:
    items: list[Any] = []
    while index < len(entries):
        line_no, line_indent, content = entries[index]
        if line_indent < indent:
            break
        if line_indent != indent:
            raise ParseError(f"{path}:{line_no}: unexpected indentation inside sequence")
        if not content.startswith("- "):
            break
        item_text = content[2:]
        index += 1
        if not item_text:
            if index >= len(entries) or entries[index][1] <= line_indent:
                raise ParseError(f"{path}:{line_no}: expected nested block after list item")
            nested, index = parse_block(path, entries, index, line_indent + 2)
            items.append(nested)
            continue
        if ":" in item_text:
            key, value, has_value = split_key_value(path, line_no, item_text)
            item: dict[str, Any] = {}
            if has_value:
                item[key] = parse_scalar(value)
            else:
                if index >= len(entries) or entries[index][1] <= line_indent:
                    raise ParseError(f"{path}:{line_no}: expected nested block for key '{key}'")
                nested, index = parse_block(path, entries, index, line_indent + 2)
                item[key] = nested
            while index < len(entries):
                next_line_no, next_indent, next_content = entries[index]
                if next_indent < line_indent + 2:
                    break
                if next_indent > line_indent + 2:
                    raise ParseError(f"{path}:{next_line_no}: unexpected indentation in list mapping")
                if next_content.startswith("- "):
                    break
                next_key, next_value, next_has_value = split_key_value(path, next_line_no, next_content)
                index += 1
                if next_has_value:
                    item[next_key] = parse_scalar(next_value)
                    continue
                if index >= len(entries) or entries[index][1] <= next_indent:
                    raise ParseError(f"{path}:{next_line_no}: expected nested block for key '{next_key}'")
                nested, index = parse_block(path, entries, index, next_indent + 2)
                item[next_key] = nested
            items.append(item)
            continue
        items.append(parse_scalar(item_text))
    return items, index


def split_key_value(path: Path, line_no: int, content: str) -> tuple[str, str, bool]:
    if ":" not in content:
        raise ParseError(f"{path}:{line_no}: expected 'key: value' entry")
    key, remainder = content.split(":", 1)
    key = key.strip()
    if not key:
        raise ParseError(f"{path}:{line_no}: empty mapping key is not supported")
    if not remainder:
        return key, "", False
    if not remainder.startswith(" "):
        raise ParseError(f"{path}:{line_no}: expected a space after ':'")
    value = remainder[1:]
    if value == "":
        return key, "", False
    return key, value, True


def parse_scalar(text: str) -> Any:
    if text == "[]":
        return []
    if text == "{}":
        return {}
    if text in {"true", "false"}:
        return text == "true"
    if text == "null":
        return None
    if text.startswith('"') and text.endswith('"'):
        return text[1:-1]
    if text.startswith("'") and text.endswith("'"):
        return text[1:-1]
    if text.startswith("-") and text[1:].isdigit():
        return int(text)
    if text.isdigit():
        return int(text)
    return text


def validate_against_schema(data: Any, schema: Any, schema_path: Path) -> None:
    if not isinstance(schema, dict):
        raise ValidationError(f"unsupported schema shape in {schema_path}")

    if "$ref" in schema:
        ref_path = schema_path.parent / schema["$ref"]
        referenced = load_yaml_like(ref_path)
        validate_against_schema(data, referenced, ref_path)
        return

    if "oneOf" in schema:
        errors = []
        for option in schema["oneOf"]:
            try:
                validate_against_schema(data, option, schema_path)
                return
            except ValidationError as exc:
                errors.append(str(exc))
        joined = "; ".join(errors)
        raise ValidationError(f"value did not match any allowed schema branch: {joined}")

    schema_type = schema.get("type")
    if schema_type == "object":
        validate_object(data, schema, schema_path)
        return
    if schema_type == "array":
        validate_array(data, schema, schema_path)
        return
    if schema_type == "string":
        validate_string(data, schema)
        return
    if schema_type == "integer":
        validate_integer(data, schema)
        return
    if schema_type == "boolean":
        if type(data) is not bool:
            raise ValidationError(f"expected boolean, found {type(data).__name__}")
        return
    if schema_type == "null":
        if data is not None:
            raise ValidationError(f"expected null, found {type(data).__name__}")
        return
    if "properties" in schema or "required" in schema:
        validate_object(data, schema, schema_path)
        return
    raise ValidationError(f"unsupported schema type '{schema_type}' in {schema_path}")


def validate_object(data: Any, schema: dict[str, Any], schema_path: Path) -> None:
    if not isinstance(data, dict):
        raise ValidationError(f"expected object, found {type(data).__name__}")

    required = schema.get("required", [])
    for key in required:
        if key not in data:
            raise ValidationError(f"missing required key '{key}'")

    properties = schema.get("properties", {})
    if schema.get("additionalProperties") is False:
        extras = sorted(set(data) - set(properties))
        if extras:
            raise ValidationError(f"unexpected keys: {', '.join(extras)}")

    for key, value in data.items():
        if key not in properties:
            continue
        try:
            validate_against_schema(value, properties[key], schema_path)
        except ValidationError as exc:
            raise ValidationError(f"{key}: {exc}") from exc


def validate_array(data: Any, schema: dict[str, Any], schema_path: Path) -> None:
    if not isinstance(data, list):
        raise ValidationError(f"expected array, found {type(data).__name__}")

    min_items = schema.get("minItems")
    if min_items is not None and len(data) < min_items:
        raise ValidationError(f"expected at least {min_items} items, found {len(data)}")

    if schema.get("uniqueItems"):
        fingerprints = {json.dumps(item, sort_keys=True) for item in data}
        if len(fingerprints) != len(data):
            raise ValidationError("expected unique items")

    item_schema = schema.get("items")
    if item_schema is not None:
        for index, item in enumerate(data):
            try:
                validate_against_schema(item, item_schema, schema_path)
            except ValidationError as exc:
                raise ValidationError(f"[{index}]: {exc}") from exc


def validate_string(data: Any, schema: dict[str, Any]) -> None:
    if not isinstance(data, str):
        raise ValidationError(f"expected string, found {type(data).__name__}")
    min_length = schema.get("minLength")
    if min_length is not None and len(data) < min_length:
        raise ValidationError(f"expected string length >= {min_length}")
    enum = schema.get("enum")
    if enum is not None and data not in enum:
        raise ValidationError(f"expected one of {enum}, found {data!r}")


def validate_integer(data: Any, schema: dict[str, Any]) -> None:
    if type(data) is not int:
        raise ValidationError(f"expected integer, found {type(data).__name__}")
    minimum = schema.get("minimum")
    if minimum is not None and data < minimum:
        raise ValidationError(f"expected integer >= {minimum}")


def validate_runtime_fixture_topology(fixture_path: Path, fixture_data: dict[str, Any], spec: RuntimeFixtureSpec) -> None:
    required_root_keys = {
        "case_name",
        "description",
        "local_runtime",
        "peer_runtime",
        "requested_scope",
        "expected_negotiation",
    }
    missing = sorted(required_root_keys - set(fixture_data))
    if missing:
        raise ValidationError(f"{fixture_path.name}: missing root keys: {', '.join(missing)}")

    actual_case_name = fixture_data["case_name"]
    expected_case_name = spec.filename.removesuffix(".yaml")
    if actual_case_name != expected_case_name:
        raise ValidationError(
            f"{fixture_path.name}: case_name expected {expected_case_name!r}, found {actual_case_name!r}"
        )

    expected_negotiation = fixture_data["expected_negotiation"]
    if not isinstance(expected_negotiation, dict):
        raise ValidationError(f"{fixture_path.name}: expected_negotiation must be an object")

    actual_status = expected_negotiation.get("status")
    if actual_status != spec.expected_status:
        raise ValidationError(
            f"{fixture_path.name}: expected status {spec.expected_status!r}, found {actual_status!r}"
        )

    forbid_collapsed_records(fixture_data, fixture_path.name)


def forbid_collapsed_records(node: Any, location: str) -> None:
    if isinstance(node, dict):
        for key, value in node.items():
            if key in FORBIDDEN_RECORD_KEYS:
                raise ValidationError(f"{location}: forbidden collapsed capability field '{key}'")
            forbid_collapsed_records(value, f"{location}.{key}")
    elif isinstance(node, list):
        for index, item in enumerate(node):
            forbid_collapsed_records(item, f"{location}[{index}]")


def validate_runtime_capability_fixtures() -> list[str]:
    runtime_schema = load_yaml_like(RUNTIME_SCHEMA_PATH)
    negotiation_schema = load_yaml_like(NEGOTIATION_SCHEMA_PATH)
    fixture_dir = REPO_ROOT / "tests" / "conformance" / "runtime-capability"

    validated_files: list[str] = []
    for spec in RUNTIME_FIXTURE_SPECS:
        fixture_path = fixture_dir / spec.filename
        fixture_data = load_yaml_like(fixture_path)
        if not isinstance(fixture_data, dict):
            raise ValidationError(f"{fixture_path.name}: fixture root must be an object")

        validate_runtime_fixture_topology(fixture_path, fixture_data, spec)
        validate_against_schema(fixture_data["local_runtime"], runtime_schema, RUNTIME_SCHEMA_PATH)
        validate_against_schema(fixture_data["peer_runtime"], runtime_schema, RUNTIME_SCHEMA_PATH)
        validate_against_schema(fixture_data["expected_negotiation"], negotiation_schema, NEGOTIATION_SCHEMA_PATH)
        validated_files.append(str(fixture_path.relative_to(REPO_ROOT)))
    return validated_files


def validate_mcu_profile_fixture_semantics(fixture_path: Path, fixture_data: dict[str, Any]) -> None:
    constraints = fixture_data.get("profile_constraints")
    if not isinstance(constraints, dict):
        raise ValidationError(f"{fixture_path.name}: profile_constraints must be an object")

    full_graph_cache_forbidden = constraints.get("full_graph_cache_forbidden")
    if full_graph_cache_forbidden is not True:
        raise ValidationError(
            f"{fixture_path.name}: profile_constraints.full_graph_cache_forbidden must be true"
        )

    if "no_heap_after_init" not in constraints:
        raise ValidationError(f"{fixture_path.name}: profile_constraints.no_heap_after_init must be explicit")

    if type(constraints["no_heap_after_init"]) is not bool:
        raise ValidationError(f"{fixture_path.name}: profile_constraints.no_heap_after_init must be boolean")

    graph_policy = fixture_data.get("graph_policy")
    if graph_policy == "full":
        raise ValidationError(f"{fixture_path.name}: constrained MCU fixtures must not claim graph_policy 'full'")


def validate_mcu_profile_fixtures() -> list[str]:
    mcu_profile_schema = load_yaml_like(MCU_PROFILE_SCHEMA_PATH)
    fixture_dir = REPO_ROOT / "tests" / "conformance" / "mcu-profiles"

    validated_files: list[str] = []
    for spec in MCU_PROFILE_FIXTURE_SPECS:
        fixture_path = fixture_dir / spec.filename
        fixture_data = load_yaml_like(fixture_path)
        if not isinstance(fixture_data, dict):
            raise ValidationError(f"{fixture_path.name}: fixture root must be an object")

        validate_against_schema(fixture_data, mcu_profile_schema, MCU_PROFILE_SCHEMA_PATH)
        validate_mcu_profile_fixture_semantics(fixture_path, fixture_data)
        validated_files.append(str(fixture_path.relative_to(REPO_ROOT)))
    return validated_files


def artifact_root() -> Path:
    configured = os.environ.get("GRCL_PLATFORM_ARTIFACT_ROOT")
    return Path(configured) if configured else DEFAULT_ARTIFACT_ROOT


def write_runtime_report(validated_files: list[str]) -> Path:
    report_dir = artifact_root() / "g5" / "schema"
    report_dir.mkdir(parents=True, exist_ok=True)
    report_path = report_dir / "runtime-capability-report.txt"
    content = [
        "runtime capability fixture validation: ok",
        f"repository_root: {REPO_ROOT}",
        f"validated_count: {len(validated_files)}",
        "validated_files:",
        *[f"- {path}" for path in validated_files],
    ]
    report_path.write_text("\n".join(content) + "\n", encoding="utf-8")
    return report_path


def write_mcu_profile_report(validated_files: list[str]) -> Path:
    report_dir = artifact_root() / "g5" / "schema"
    report_dir.mkdir(parents=True, exist_ok=True)
    report_path = report_dir / "mcu-profiles-report.txt"
    content = [
        "mcu profile fixture validation: ok",
        f"repository_root: {REPO_ROOT}",
        f"validated_count: {len(validated_files)}",
        "validated_files:",
        *[f"- {path}" for path in validated_files],
    ]
    report_path.write_text("\n".join(content) + "\n", encoding="utf-8")
    return report_path


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv or sys.argv[1:])

    try:
        if args.runtime_capability:
            validated_files = validate_runtime_capability_fixtures()
            report_path = write_runtime_report(validated_files)
            print(f"runtime capability fixtures valid: {len(validated_files)}")
            print(f"report: {report_path}")
            return 0

        if args.mcu_profiles:
            validated_files = validate_mcu_profile_fixtures()
            report_path = write_mcu_profile_report(validated_files)
            print(f"mcu profile fixtures valid: {len(validated_files)}")
            print(f"report: {report_path}")
            return 0

        raise ValidationError("no validation target selected")
    except (ParseError, ValidationError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
