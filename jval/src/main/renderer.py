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

from pathlib import Path
from jinja2 import Environment, FileSystemLoader, Template
from compiler import RenderUnit
from typing import List, Dict
from dataclasses import dataclass, asdict

home_path = Path(__file__).resolve() # Full absolute path to where the compiler lives, not necessarily the CWD

# Converts a render unit into a dictionary that can be read by Jinja
def prerender(runit: RenderUnit) -> Dict:
    runit_dict = asdict(runit)
    # Null values are pruned from the dictionary, as the Jinja template was designed under the assumption that
    # missing values are completely absent from the input dictionary
    if isinstance(runit_dict["schema"],Dict):
        schema = runit_dict["schema"]
        for key in list(schema.keys()):
            if schema[key] is None:
                del schema[key]
    
    for anon in runit_dict["anonymousSchemas"]:
        anon_schema = anon["schema"]
        for key in list(anon_schema.keys()):
            if anon_schema[key] is None:
                del anon_schema[key]
    
    return runit_dict

@dataclass
class RenderTarget:
    path: Path
    template: Template
    renderDict: Dict

def copyfile(src: Path, dest: Path):
    with open(src,'r') as f_src:
        content = f_src.read()
    with open(dest,'w') as f_dest:
        f_dest.write(content)

def render(renderUnits: List[RenderUnit],targetDirectory: Path):
    print("Starting render...")
    env = Environment(loader=FileSystemLoader(home_path.parent.parent / "templates"))
    capi_template = env.get_template("capi.jval.h.jinja")
    header_template = env.get_template("header.jval.hpp.jinja")
    source_template = env.get_template("source.jval.cpp.jinja")
    renderDicts: List[Dict] = []
    for renderUnit in renderUnits: 
        renderDicts.append(prerender(renderUnit))
    
    renderTargets: List[RenderTarget] = []
    for renderDict in renderDicts:
        name = renderDict["schema"]["name"]
        renderTargets.append(RenderTarget(
            targetDirectory / f"{name}_capi.jval.h",
            capi_template,
            renderDict
        ))
        renderTargets.append(RenderTarget(
            targetDirectory / f"{name}.jval.hpp",
            header_template,
            renderDict
        ))
        renderTargets.append(RenderTarget(
            targetDirectory / f"{name}.jval.cpp",
            source_template,
            renderDict
        ))
    
    for renderTarget in renderTargets:
        print(f"Rendering target {renderTarget.path}")
        rendered_code = renderTarget.template.render(renderTarget.renderDict)
        with open(renderTarget.path,'w') as f:
            f.write(rendered_code)

    print("Render complete")
    print("Loading JVal Runtime")
    copyfile(home_path.parent.parent/"runtime"/"jvruntime.cpp",targetDirectory/"jvruntime.cpp")
    copyfile(home_path.parent.parent/"runtime"/"jvruntime.hpp",targetDirectory/"jvruntime.hpp")
    copyfile(home_path.parent.parent/"runtime"/"jv_capi.h",targetDirectory/"jv_capi.h")
    copyfile(home_path.parent.parent/"runtime"/"jvexport.h",targetDirectory/"jvexport.h")

    
    