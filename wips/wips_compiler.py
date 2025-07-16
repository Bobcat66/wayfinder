# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2025 Jesse Kane
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

from jinja2 import Environment, FileSystemLoader
import argparse
import yaml
from typing import List, Dict, Set, Tuple, Self
from pathlib import Path
import time

home_path = Path(__file__).resolve()    # Full absolute path to where the compiler lives, not necessarily the CWD

def get_ctype_name(field: Dict) -> str:
    return f"wips_{field["type"]}_t"

def verify_yaml_schemas(yaml_schemas: List[Dict]) -> Tuple[int,str]:
    defined_types: Set[str] = set(["u8","i8","u16","i16","u32","i32","u64","i64","fp32","fp64"]) # The types that have already been defined
    for schema in yaml_schemas:
        name: str = schema["name"]
        declared_fields: Set[str] = set()
        for field in schema["fields"]:
            if not field["type"] in defined_types:
                return 1, f"{name} field {field} is declared as undefined type {field["type"]}" # One denotes type error
            if field["name"] in declared_fields:
                return 2, f"field {field} is declared twice in {name}"
            declared_fields.add(field["name"])
        defined_types.add(name)
    return 0, "YAML schema file verified"

class wips_schema:

    def __init__(self,yaml_schema: Dict,dependencies: List[Self]):
        self.name: str = yaml_schema["name"]
        self.fields: List[Dict] = yaml_schema["fields"]
        self.preprocessed: bool = False
        self.trivial: bool = True
        self.dependencies = dependencies
    
    def preprocess(self) -> None:
        if self.preprocessed:
            return
        detail_fields: Dict[int,Dict] = {}
        for index, field in enumerate(self.fields):
            if field.get("optional"):
                detail_fields[index] = {"name":f"DETAILoptpresent__{field["name"]}","type":"u8","trivial":True}
                self.trivial = False
            elif field.get("vla"):
                detail_fields[index] = {"name":f"DETAILvlasize__{field["name"]}","type":"u32","trivial":True}
                self.trivial = False
            else:
                continue
        offset: int = 0
        for index, detail_field in detail_fields.items():
            self.fields.insert(index + offset,detail_field)
            offset += 1
        for dependency in self.dependencies:
            if not dependency.trivial:
                self.trivial = False
        self.preprocessed = True

def build_schemas(yaml_schemas: List[Dict]):
    print("Building schemas...")
    primitives = set(["u8","i8","u16","i16","u32","i32","u64","i64","fp32","fp64"])
    schemas: Dict[str,wips_schema] = {}
    for yaml_schema in yaml_schemas:
        print(f"Building {yaml_schema["name"]} schema")
        dependency_names: Set[str] = set()
        for field in yaml_schema["fields"]:
            if field["type"] in primitives:
                field["trivial"] = True
                continue
            elif field["type"] in schemas:
                field["trivial"] = schemas[field["type"]].trivial
            dependency_names.add(field["type"])
        dependencies: List[wips_schema] = []
        for dependency_name in dependency_names:
            dependencies.append(schemas[dependency_name])
        schema = wips_schema(yaml_schema,dependencies)
        schema.preprocess()
        schemas[schema.name] = schema
    print("Built schemas")
    return schemas

def copyfile(src: Path, dest: Path):
    with open(src,'r') as f_src:
        content = f_src.read()
    with open(dest,'w') as f_dest:
        f_dest.write(content)

def compile(schemas_path: Path, output_dir: Path, py: bool = False, jvm: bool = False) -> None:
    start = time.perf_counter()
    print(f"Compiling {schemas_path}...")
    # Parse YAML
    with open(schemas_path, "r") as f:
        yaml_schemas = list(yaml.safe_load_all(f))

    err, msg = verify_yaml_schemas(yaml_schemas)
    if err:
        print(f"Error {err}: {msg}")
        return
    print(msg)
    # Build Schemas
    schemas = build_schemas(yaml_schemas)
    # Render code
    env = Environment(loader=FileSystemLoader(home_path.parent / "templates"))
    env.filters["get_ctype_name"] = get_ctype_name
    header_template = env.get_template("header.wips.h.jinja")
    source_template = env.get_template("source.wips.c.jinja")
    render_targets: Dict[Path,Dict[str,any]] = {}
    for schema in schemas.values():
        render_targets[output_dir / f"{schema.name}.wips.h"] = {
            "template" : header_template,
            "params" : {
                "name" : schema.name,
                "trivial" : schema.trivial,
                "fields" : schema.fields,
                "dependencies" : schema.dependencies
            }
        }
        render_targets[output_dir / f"{schema.name}.wips.c"] = {
            "template" : source_template,
            "params" : {
                "name" : schema.name,
                "trivial" : schema.trivial,
                "fields" : schema.fields,
                "dependencies" : schema.dependencies
            }
        }
    print("Rendering code...")
    for render_target,render_config in render_targets.items():
        print(f"Rendering target {render_target}")
        rendered_code = render_config["template"].render(**render_config["params"])
        with open(render_target,'w') as f:
            f.write(rendered_code)
    print("Render complete")
    # Loading runtime
    print("Loading WIPS runtime")
    copyfile(home_path.parent/"runtime"/"wips_runtime.c",output_dir/"wips_runtime.c")
    copyfile(home_path.parent/"runtime"/"wips_runtime.h",output_dir/"wips_runtime.h")
    copyfile(home_path.parent/"runtime"/"wips_detail.h",output_dir/"wips_detail.h")
    end = time.perf_counter()
    print(f"Compilation finished in {end-start} seconds")

# Core:
# python3 wips_compiler.py messages.yaml --out=../core/src/generated/wips

# Client:
# python3 wips_compiler.py messages.yaml --py --out=../client/src/generated/wips

# Lib:
# python3 wips_compiler.py messages.yaml --jvm --out=../wayfinderlib/src/generated/wips

# Testing:
# python3 wips_compiler.py messages.yaml --out=./wips_output

# Python and Java bindings generation is not implemented yet, but the flags are here for future use.
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="WIPS Compiler")
    parser.add_argument("schemas", type=Path, help="path to YAML schema file")
    parser.add_argument("--out", type=Path, default=".", help='Output directory')
    parser.add_argument("--py", action="store_true", help="Generate python bindings for the C code")
    parser.add_argument("--jvm", action="store_true", help="Generate Java bindings for the C code")
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    compile(args.schemas,args.out,args.py,args.jvm)

