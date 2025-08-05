# WIPS

WIPS is wayfinder's internal serialization-deserialization library.
The code generator will emit a header file and an implementation file for each message type specified in the 
YAML file.

> IMPORTANT: WIPS user-defined types cannot be named the same as any of the primitives, and they also cannot be named "bin". WIPS supports optionals and variable-length arrays, but does NOT support optional variable-length arrays or variable-length arrays of optionals (at least not directly; you theoretically should be able to wrap a VLA or an optional inside a WIPS type and then use those freely. However, this has not been tested).

> IMPORTANT: WIPS's debugging code relies on the VA_OPT preprocesser flag, which apparently might be broken on
MSVC (A cursory google search turns up inconsistent and contradictory results). This shouldn't cause problems, as 
GCC (which does support VA_OPT) is the primary compiler used during testing and debugging for both WIPS and 
Wayfinder as a whole, and when built in release mode WIPS does not make use of VA_OPT at all. Nevertheless, it's 
worth noting for posterity.

Note: in general, the values returned by getters should be treated as invalidated after the original object is destroyed or modified