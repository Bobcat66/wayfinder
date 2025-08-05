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

from typing import List, Dict, Set, Tuple, Self, Union
from pathlib import Path
from dataclasses import dataclass

primitiveValidators: Dict[str,str] = {
    "boolean": "getPrimitiveValidator<bool>()",
    "number": "getPrimitiveValidator<double>()",
    "integer": "getPrimitiveValidator<int>()",
    "string": "getPrimitiveValidator<std::string>()",
    "any": "getNullValidator()",
}

@dataclass
class Reference:
    name: str
    validator: str

@dataclass
class Primitive:
    type: str

@dataclass
class DeepSchema:
    name: str
    type: str
    properties: Union[Dict[str,Union[Self,Reference,Primitive]],None]
    required: Union[Set[str],None]
    dependencies: Union[Dict[str,Set[str]],None]
    enumValues: Union[Set[str],None]
    schemas: Union[List[Union[Self,Reference,Primitive]],None]
    regex: Union[str,None]
    mapKeys: Union[str,None]
    mapValues: Union[Self,Reference,Primitive,None]
    items: Union[Self,Reference,Primitive,None]
    minSize: Union[int,None]
    maxSize: Union[int,None]

# A representation of a schema that has been fully flattened, so that there are no anonymous
# inner schemas, and all schemas are represented by strings of valid C++ that invoke the
# corresponding validation functor getter. FlatSchemas are ready for rendering with Jinja 
@dataclass
class FlatSchema:
    name: str
    type: str
    properties: Union[Dict[str,str],None]
    required: Union[Set[str],None]
    dependencies: Union[Dict[str,Set[str]],None]
    enumValues: Union[Set[str],None]
    schemasValidators: Union[Set[str],None]
    regex: Union[str,None]
    mapKeys: Union[str,None]
    mapValueValidator: Union[str,None]
    itemsValidator: Union[str,None]
    minSize: Union[int,None]
    maxSize: Union[int,None]

@dataclass
class AnonymousSchema:
    schema: FlatSchema
    fwd_decls: List[str]

# A schema module is an intermediate representation of a single
# JVal schema file, with all anonymous schemas and references resolved
@dataclass
class SchemaModule:
    schema: Union[FlatSchema,Primitive,Reference]
    anonymousSchemas: List[AnonymousSchema]
    includes: List[str]

@dataclass
class SchemaTree:
    schema: FlatSchema
    children: List[Self]
    
home_path = Path(__file__).resolve() # Full absolute path to where the compiler lives, not necessarily the CWD

def manglePath(path: List[str]) -> str:
    # _z42D is a special identifier that marks a name mangled by JVal
    return "_z42D" + "_".join(path)

# path is a json pointer corresponding to the field in the jval schema file where this schema dict is found
def unpackSchemaDict(schemaDict: Dict,refs: Dict[str,str], path: List[str], mangledName: bool = True) -> Tuple[Union[DeepSchema,Primitive,Reference],List[Reference]]:
    name: str
    if mangledName:
        name = manglePath(path)
    else:
        name = schemaDict["$name"]

    if "$ref" in schemaDict:
        # this schema dict is a reference
        refname = schemaDict["$ref"]
        if refname in refs:
            ret = Reference(
                refname,
                refs[refname]
            )
            return ret,[ret]
        else:
            raise RuntimeError(f"{name} references unknown schema '{refname}'")
    
    localRefs: List[Reference] = []
        
    # extraction of top level
    schema_type: str = schemaDict["type"]
    raw_properties: Union[Dict[str,Dict],None] = schemaDict.get("properties",None)
    raw_required: Union[List[str],None] = schemaDict.get("required",None)
    raw_dependencies: Union[Dict[str,List[str]],None] = schemaDict.get("dependencies",None)
    raw_enumValues: Union[List[str],None] = schemaDict.get("enumValues",None)
    raw_schemas: Union[List[Dict],None] = schemaDict.get("schemas",None)
    regex: Union[str,None] = schemaDict.get("regex",None)
    mapKeys: Union[str,None] = schemaDict.get("mapKeys",None)
    raw_mapValues: Union[Dict,None] = schemaDict.get("mapValues",None)
    raw_items: Union[Dict,None] = schemaDict.get("items",None)
    minSize: Union[int,None] = schemaDict.get("minSize",None)
    maxSize: Union[int,None] = schemaDict.get("maxSize",None)

    match schema_type:
        case "struct":
            # Process properties, requires, and dependencies
            properties: Union[Dict[str,Union[DeepSchema,Reference,Primitive]],None] = None
            if raw_properties is not None:
                properties = {}
                for propName,propVal in raw_properties.items():
                    properties[propName],temprefs = unpackSchemaDict(propVal,refs,path+[propName])
                    localRefs += temprefs
            required: Union[Set[str],None] = None
            if raw_required is not None:
                required = set(raw_required)
            dependencies: Union[Dict[str,Set[str]],None] = None
            if raw_dependencies is not None:
                dependencies = {}
                for dependency,dependents in raw_dependencies.items():
                    dependencies[dependency] = set(dependents)
            return DeepSchema(
                name,
                schema_type,
                properties,
                required,
                dependencies,
                None,None,None,None,
                None,None,None,None
            ),localRefs
        
        case "array":
            # process items, minSize, and maxSize
            items: Union[DeepSchema,Primitive,Reference,None] = None
            if raw_items is not None:
                items,temprefs = unpackSchemaDict(raw_items,refs,path + ["items"])
                localRefs += temprefs
            return DeepSchema(
                name,
                schema_type,
                None,None,None,None,
                None,None,None,None,
                items,
                minSize,
                maxSize
            ),localRefs

        case "enum":
            enumValues: Union[Set[str],None] = None
            if raw_enumValues is not None:
                enumValues = set(raw_enumValues)
            return DeepSchema(
                name,
                schemaDict,
                None,None,None,
                enumValues,
                None,None,None,
                None,None,None,
                None
            ),localRefs
        
        case "union":
            # process schemas
            schemas: Union[List[Union[DeepSchema,Reference,Primitive]],None] = None
            if raw_schemas is not None:
                schemas = []
                for i in range(0,len(raw_schemas)):
                    tempDS,temprefs = unpackSchemaDict(raw_schemas[i],refs,path+[f"{i}"])
                    schemas.append(tempDS)
                    localRefs += temprefs
            return DeepSchema(
                name,
                schema_type,
                None,None,None,None,
                schemas,
                None,None,None,None,
                None,None
            ),localRefs
        
        case "map":
            # process mapKeys and mapValues
            mapValues: Union[DeepSchema,Reference,Primitive,None] = None
            if raw_mapValues is not None:
                mapValues,temprefs = unpackSchemaDict(mapValues,refs,path+["mapValues"])
                localRefs += temprefs
            return DeepSchema(
                name,
                schema_type,
                None,None,None,
                None,None,None,
                mapKeys,mapValues,
                None,None,None
            ),localRefs
        
        case "pattern":
            # process regex
            return DeepSchema(
                name,
                schema_type,
                None,None,None,
                None,None,
                regex,
                None,None,None,
                None,None
            ),localRefs
        
        case "boolean": return Primitive("boolean"),localRefs
        case "integer": return Primitive("integer"),localRefs
        case "number": return Primitive("number"),localRefs
        case "string": return Primitive("string"),localRefs
        case "any": return Primitive("any"),localRefs

def getDeepSchemaValidator(schema: DeepSchema) -> str:
    return f"get_{schema.name}_validator()"

def getPrimitiveValidator(schema: Primitive) -> str:
    return primitiveValidators[schema.type]

def getReferenceValidator(schema: Reference) -> str:
    return schema.validator

def buildTree(schema: DeepSchema) -> SchemaTree:
    deepChildren: List[DeepSchema] = []
    # Flatten main schema

    flatProperties: Union[Dict[str,str],None] = None
    if schema.properties is not None:
        for property,value in schema.properties.items():
            if isinstance(value,DeepSchema):
                deepChildren.append(value)
                flatProperties[property] = getDeepSchemaValidator(value)
                continue
            if isinstance(value,Reference):
                flatProperties[property] = getReferenceValidator(value)
                continue
            flatProperties[property] = getPrimitiveValidator(value)
    
    flatSchemas: Union[List[str],None] = None
    if schema.schemas is not None:
        flatSchemas = []
        for union_schema in schema.schemas:
            if isinstance(union_schema,DeepSchema):
                deepChildren.append(union_schema)
                flatSchemas.append(getDeepSchemaValidator(union_schema))
                continue
            if isinstance(union_schema,Reference):
                flatSchemas.append(getReferenceValidator(union_schema))
                continue
            flatSchemas.append(getPrimitiveValidator(union_schema))
    
    flatMapValue: Union[str,None] = None
    if schema.mapValues is not None:
        if isinstance(schema.mapValues,DeepSchema):
            deepChildren.append(schema.mapValues)
            flatMapValue = getDeepSchemaValidator(schema.mapValues)
        elif isinstance(schema.mapValues,Reference):
            flatMapValue = getReferenceValidator(schema.mapValues)
        else:
            flatMapValue = getPrimitiveValidator(schema.mapValues)
    
    flatItems: Union[str,None] = None
    if schema.items is not None:
        if isinstance(schema.items,DeepSchema):
            deepChildren.append(schema.items)
            flatItems = getDeepSchemaValidator(schema.items)
        elif isinstance(schema.items,Reference):
            flatItems = getReferenceValidator(schema.items)
        else:
            flatItems = getPrimitiveValidator(schema.items)

    flatSchema: FlatSchema = FlatSchema(
        schema.name,
        schema.type,
        flatProperties,
        schema.required,
        schema.dependencies,
        schema.enumValues,
        flatSchemas,
        schema.regex,
        schema.mapKeys,
        flatMapValue,
        flatItems,
        schema.minSize,
        schema.maxSize
    )

    flatChildren = [buildTree(deepChild) for deepChild in deepChildren]
    return SchemaTree(
        flatSchema,
        flatChildren
    )

def getFwdDecl(schema: SchemaTree) -> str:
    return f"get_{schema.schema.name}_validator()"

def getAnonymousSchemas(schema: SchemaTree,isAnonymous: bool = True) -> List[AnonymousSchema]:
    res: List[AnonymousSchema] = []
    if isAnonymous:
        fwd_decls: List[str] = [getFwdDecl(child) for child in schema.children]
        res.append(AnonymousSchema(
            schema.schema,
            fwd_decls
        ))
    for child in schema.children:
        res.append(getAnonymousSchemas(child))
    return res

def compileSchemas(schemas: List[Dict]) -> List[SchemaModule]:
    refs: Dict[str,str] = {}
    modules: List[SchemaModule] = []
    # build reference table
    for schema in schemas:
        refs[schema] = f"get_{schema["$name"]}_validator()"
    
    for schema in schemas:
        schema_obj,schema_refs = unpackSchemaDict(schema,refs,["root"],mangledName=False)
        if isinstance(schema_obj,DeepSchema):
            schema_tree = buildTree(schema_obj)
            anons = getAnonymousSchemas(schema_tree,isAnonymous=False)
            modules.append(SchemaModule(schema_tree.schema,anons,[ref.name for ref in schema_refs]))
            continue
        elif isinstance(schema_obj,Reference):
            modules.append(SchemaModule(schema_obj,[],[schema_obj.name]))
            continue
        else:
            modules.append(SchemaModule(schema_obj,[],[]))
            continue

