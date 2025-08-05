import argparse
from pathlib import Path
from parser import loadSchemas
from compiler import compileSchemas
from renderer import render

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="JVal Compiler")
    parser.add_argument("--out", type=Path, default=".", help='Output directory')
    parser.add_argument("inputs", nargs="*",type=Path, help="List of input files")
    args = parser.parse_args()
    schemaDicts = loadSchemas(args.inputs)
    renderUnits = compileSchemas(schemaDicts)
    args.out.mkdir(parents=True, exist_ok=True)
    render(renderUnits,args.out)

