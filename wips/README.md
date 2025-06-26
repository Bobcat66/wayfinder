# WIPS

WIPS is wayfinder's internal serialization-deserialization library.
The code generator will emit a header file and an implementation file.
> IMPORTANT: WIPS user-defined types cannot be named the same as any of the primitives, and they also cannot be named "bin". WIPS supports optionals and variable-length arrays, but does NOT support optional variable-length arrays or variable-length arrays of optionals (at least not directly; you theoretically should be able to wrap a VLA or an optional inside a WIPS type and then use those freely. However, this has not been tested)