# SourceMod JSON Extension

## Overview
A [SourceMod](http://www.sourcemod.net/) extension that provides comprehensive JSON manipulation capabilities, powered by [YYJSON](https://github.com/ibireme/yyjson).

## Key Features
* High-performance JSON parsing and serialization using YYJSON
* Support for [JSON Pointer](https://datatracker.ietf.org/doc/html/rfc6901) operations
* x64 support
* Easy-to-use API for both objects and arrays
* Pretty printing and writing support
* Array and object sorting support
* Iteration methods for arrays and objects

## Building from Source
```bash
# Clone the repository
git clone https://github.com/ProjectSky/sm-ext-yyjson.git
cd sm-ext-yyjson

# Create build directory
mkdir build && cd build

# Configure and build
python ../configure.py --enable-optimize --symbol-files \
    --sm-path=YOUR_SOURCEMOD_PATH \
    --targets=x64,x86
ambuild
```

## Documentation
* [API Reference](https://github.com/ProjectSky/sm-ext-yyjson/blob/main/scripting/include/yyjson.inc)
* [Latest Builds](https://github.com/ProjectSky/sm-ext-yyjson/actions)

## Usage Examples

### Working with Objects
```cpp
// Creating and manipulating JSON objects
YYJSONObject obj = new YYJSONObject();
obj.SetInt("int", 1);
obj.SetInt64("int64", "9223372036854775800");
obj.SetFloat("float", 2.0);
obj.SetBool("bool", true);
obj.SetString("str", "Hello World");
obj.SetNull("null");
delete obj;

/* Output:
{
  "int": 1,
  "int64": 9223372036854775800,
  "float": 2.0,
  "bool": true,
  "str": "Hello World",
  "null": null
}
*/
```

### Working with Arrays
```cpp
// Creating and manipulating JSON arrays
YYJSONArray arr = new YYJSONArray();
arr.PushInt(1);
arr.PushInt64("9223372036854775800");
arr.PushFloat(2.0);
arr.PushBool(true);
arr.PushString("Hello World");
arr.PushNull();
delete arr;

/* Output:
[
  1,
  9223372036854775800,
  2.0,
  true,
  "Hello World",
  null
]
*/
```

### Using JSON Pointer
```cpp
// Creating nested structures
YYJSONObject obj = new YYJSONObject();
obj.PtrSetInt("/a/b/c", 1);
delete obj;

/* Output:
{
  "a": {
    "b": {
      "c": 1
    }
  }
}
*/

// Querying data
YYJSONObject data = YYJSON.Parse("example.json", true);
int value = data.PtrGetInt("/int");        // Get value: 1234
float fValue = data.PtrGetFloat("/arr/1"); // Get value: 1.2344
delete data;
```

### Iteration Examples

#### Object Iteration
```cpp
char key[64];
YYJSON value;

// Method 1: Using Foreach (Recommended)
while (obj.ForeachObject(key, sizeof(key), value))
{
  PrintToServer("Key: %s", key);
  // Process value
  delete value;
}

// Method 2: Classic iteration
for (int i = 0; i < obj.Size; i++)
{
  obj.GetKey(i, key, sizeof(key));
  value = obj.GetValueAt(i);
  // Process value
  delete value;
}
```

#### Array Iteration
```cpp
YYJSONArray arr = obj.PtrGet("/arr");
YYJSON value;
int index;

// Method 1: Using Foreach (Recommended)
while (arr.ForeachArray(index, value))
{
  // Process value
  delete value;
}

// Method 2: Classic iteration
for (int i = 0; i < arr.Length; i++)
{
  value = arr.Get(i);
  // Process value
  delete value;
}
delete arr;
```

### Sorting Arrays and Objects
```cpp
// Sorting JSON arrays
YYJSONArray arr = YYJSON.Parse(
  "[3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5]"
);
arr.Sort(); // Ascending order (default)
// [1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9]

arr.Sort(YYJSON_SORT_DESC); // Descending order
// [9, 6, 5, 5, 5, 4, 3, 3, 2, 1, 1]

// Arrays with mixed types are sorted by type first, then by value
YYJSONArray mixed = YYJSON.Parse(
  "[true, 42, \"hello\", 1.23, false, \"world\"]"
);
mixed.Sort();
// [false, true, 1.23, 42, "hello", "world"]

// Sorting JSON objects by keys
YYJSONObject obj = YYJSON.Parse(
  "{\"zebra\": 1, \"alpha\": 2, \"beta\": 3, \"gamma\": 4}"
);

obj.Sort(); // Ascending order by keys (default)
// {"alpha": 2, "beta": 3, "gamma": 4, "zebra": 1}

obj.Sort(YYJSON_SORT_DESC); // Descending order by keys
// {"zebra": 1, "gamma": 4, "beta": 3, "alpha": 2}

delete arr;
delete mixed;
delete obj;
```

## Notes
* Beta version - some functions may require additional testing
* Please report any issues on GitHub
* Contributions are welcome