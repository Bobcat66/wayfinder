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

import json
from jsonschema import Draft7Validator, ValidationError
from pathlib import Path
from typing import List, Dict
import sys
home_path = Path(__file__).resolve() # Full absolute path to where the compiler lives, not necessarily the CWD

with open(home_path.parent.parent.parent / "jval_schema.schema.json","r") as f:
    metaschema = json.load(f)

metavalidator = Draft7Validator(metaschema)

def loadSchemas(paths: List[Path]) -> List[Dict]:
    schemas: List[Dict] = []
    for path in paths:
        with open(path,"r") as f:
            schema = json.load(f)
        errors = sorted(metavalidator.iter_errors(schema), key=lambda e: e.path)
        if errors:
            for error in errors:
                print(f"Validation error: {error.message}")
            sys.exit(1)
        schemas.append(schema)
    return schemas
        
        
        

