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
* Support for both mutable and immutable JSON documents
* Support SourceMod Extension API

## Performance
Performance test results using [twitter.json](https://github.com/ibireme/yyjson_benchmark/blob/master/data/json/twitter.json) (0.60 MB):

```
=== YYJSON Performance Benchmark ===
Test iterations: 100
Data size: 0.60 MB
Parse time: 0.025 seconds
Stringify time: 0.013 seconds
Parse operations per second: 3937.93 ops/sec
Parse speed: 2371.66 MB/s (2.31 GB/s)
Stringify speed: 4505.23 MB/s (4.39 GB/s)
Stringify operations per second: 7480.55 ops/sec
=== YYJSON Performance Benchmark End ===
```

Test environment:
- OS: Ubuntu 22.04
- CPU: AMD Ryzen 9 7950X3D
- Test data: [twitter.json](https://github.com/ibireme/yyjson_benchmark/blob/master/data/json/twitter.json)
- Test iterations: 100
- SourceMod Version: 1.13.0.6966
- YYJSON Version: Latest version
- Test script: [yyjson_perf_test.sp](scripting/yyjson_perf_test.sp)

Note: Performance may vary depending on server hardware and load conditions.

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
* [Latest Release](https://github.com/ProjectSky/sm-ext-yyjson/releases)

### SourceMod Extension API
```cpp
#include <IYYJSONManager.h>

IYYJSONManager* g_pYYJSONManager = nullptr;

void Ext::SDK_OnAllLoaded()
{
  SM_GET_LATE_IFACE(YYJSONMANAGER, g_pYYJSONManager);

  char error[256];
  YYJSONValue* val = g_pYYJSONManager->ParseJSON("{\"name\":\"John\", \"age\":30}", false, false, 0, error, sizeof(error));

  if (!val) {
    PrintToServer("Failed to parse JSON: %s", error);
    return;
  }

  size_t size = g_pYYJSONManager->GetSerializedSize(val);
  char buffer[size];
  g_pYYJSONManager->WriteToString(val, buffer, size);

  // must release the value after using
  g_pYYJSONManager->Release(val);

  printf("JSON: %s\n", buffer);
}
```

### Basic Examples

#### Working with Objects
```cpp
// Create a JSON object
YYJSONObject obj = new YYJSONObject();
obj.SetInt("int", 1);
obj.SetInt64("int64", "9223372036854775800");
obj.SetFloat("float", 2.0);
obj.SetBool("bool", true);
obj.SetString("str", "Hello World");
obj.SetNull("null");

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

delete obj;
```

#### Working with Arrays
```cpp
// Create a JSON array
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

### Advanced Features

#### Using JSON Pointer
```cpp
// Create nested structures
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

/* example.json:
{
  "int": 1234,
  "arr": [1, 1.2344, 3],
  "nested": {
    "obj": {
      "value": 42
    }
  }
}
*/

YYJSONObject data = YYJSON.Parse("example.json", true);
// Access values using JSON Pointer
int value = data.PtrGetInt("/int"); // Returns: 1234
float fValue = data.PtrGetFloat("/arr/1"); // Returns: 1.2344
int nested = data.PtrGetInt("/nested/obj/value"); // Returns: 42
delete data;
```

#### Array and Object Iteration
```cpp
// Object iteration
YYJSONObject obj = YYJSON.Parse("{\"a\": 1, \"b\": 2, \"c\": 3}");
char key[64];
YYJSON value;

// Method 1: Using Foreach (Recommended)
while (obj.ForeachObject(key, sizeof(key), value)) {
  PrintToServer("Key: %s", key);
  delete value;
}

// Method 2: Using ForeachKey (only used for keys)
while (obj.ForeachKey(key, sizeof(key))) {
  PrintToServer("Key: %s", key);
}

// Method 3: Classic iteration
for (int i = 0; i < obj.Size; i++) {
  obj.GetKey(i, key, sizeof(key));
  value = obj.GetValueAt(i);
  delete value;
}

delete obj;

// Array iteration
YYJSONArray arr = YYJSON.Parse("[1, 2, 3, 4, 5]");
int index;

// Method 1: Using Foreach (Recommended)
while (arr.ForeachArray(index, value)) {
  PrintToServer("Index: %d", index);
  delete value;
}

// Method 2: Using ForeachIndex (only used for index)
while (arr.ForeachIndex(index)) {
  PrintToServer("Index: %d", index);
}

// Method 3: Classic iteration
for (int i = 0; i < arr.Length; i++) {
  value = arr.Get(i);
  delete value;
}

delete arr;
```

#### Array Search Operations
```cpp
// Create a test array
YYJSONArray arr = YYJSON.Parse(
  "[42, true, \"hello\", 3.14, \"world\", false, 42]"
);

// Search for values (returns first occurrence)
int index;
index = arr.IndexOfInt(42);           // Returns 0
index = arr.IndexOfBool(true);        // Returns 1
index = arr.IndexOfString("hello");   // Returns 2
index = arr.IndexOfFloat(3.14);       // Returns 3
index = arr.IndexOfString("world");   // Returns 4
index = arr.IndexOfBool(false);       // Returns 5

// Search for non-existent values
index = arr.IndexOfInt(999);          // Returns -1
index = arr.IndexOfString("missing"); // Returns -1
index = arr.IndexOfFloat(2.718);      // Returns -1

delete arr;
```

#### Sorting Arrays and Objects
```cpp
// Array sorting
YYJSONArray arr = YYJSON.Parse(
  "[3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5]", .is_mutable_doc = true
);

arr.Sort(); // Ascending (default)
// [1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9]

arr.Sort(YYJSON_SORT_DESC); // Descending
// [9, 6, 5, 5, 5, 4, 3, 3, 2, 1, 1]

arr.Sort(YYJSON_SORT_RANDOM); // Random
// [5, 2, 9, 1, 6, 3, 4, 5, 1, 3, 5] (example output)

// Mixed type array sorting
YYJSONArray mixed = YYJSON.Parse(
  "[true, 42, \"hello\", 1.23, false, \"world\"]", .is_mutable_doc = true
);
mixed.Sort();
// [false, true, 1.23, 42, "hello", "world"]

// Object sorting by keys
YYJSONObject obj = YYJSON.Parse(
  "{\"zebra\": 1, \"alpha\": 2, \"beta\": 3, \"gamma\": 4}", .is_mutable_doc = true
);

obj.Sort(); // Ascending (default)
// {"alpha": 2, "beta": 3, "gamma": 4, "zebra": 1}

obj.Sort(YYJSON_SORT_DESC); // Descending
// {"zebra": 1, "gamma": 4, "beta": 3, "alpha": 2}

obj.Sort(YYJSON_SORT_RANDOM); // Random
// {"beta": 3, "zebra": 1, "alpha": 2, "gamma": 4} (example output)

delete arr;
delete mixed;
delete obj;
```

#### Using FromStrings
```cpp
// Create object from key-value string arrays
char pairs[][] = {"name", "test", "type", "demo", "version", "1.0.0"};

YYJSONObject obj = YYJSONObject.FromStrings(pairs, sizeof(pairs));

/* Output:
{
  "name": "test",
  "type": "demo",
  "version": "1.0.0"
}
*/

// Create array from string array
char items[][] = {"apple", "banana", "orange"};
YYJSONArray arr = YYJSONArray.FromStrings(items, sizeof(items));
delete obj;
delete arr;

/* Output:
[
  "apple",
  "banana",
  "orange"
]
*/
```

#### Using JSON Pack
```cpp
// Create object with mixed types
YYJSON packed = YYJSON.Pack("{s:s,s:i,s:f,s:b,s:n}",
  "name", "John",
  "age", 25,
  "height", 1.75,
  "active", true,
  "extra"
);
delete packed;

/* Output:
{
  "name": "John",
  "age": 25,
  "height": 1.75,
  "active": true,
  "extra": null
}
*/

// Create nested structures
YYJSON nested = YYJSON.Pack("{s:{s:s,s:[iii]}}",
  "user",
    "name", "John",
    "scores", 85, 90, 95
);
delete nested;

/* Output:
{
  "user": {
    "name": "John",
    "scores": [85, 90, 95]
  }
}
*/

// Create array with mixed types
YYJSON array = YYJSON.Pack("[sifbn]",
    "test", 42, 3.14, true
);
delete array;

/* Output:
[
  "test",
  42,
  3.14,
  true,
  null
]
*/
```

## Working with Immutable Documents
When parsing JSON documents, you can choose whether to create a mutable or immutable document:

```cpp
// Create an immutable document (read-only)
YYJSONObject obj = YYJSON.Parse("example.json", true);

// Create a mutable document (read-write)
YYJSONObject obj = YYJSON.Parse("example.json", true, true);
```

Immutable documents:
* Are read-only and cannot be modified
* Use less memory
* Throw errors when attempting modification operations

### Operations on Immutable Documents
Immutable documents support a variety of read operations:

- **Type Checking**: You can check the type of values within the document.
- **Value Retrieval**: You can retrieve values using keys or indices.
- **Iteration**: You can iterate over arrays and objects.
- **Comparison**: You can compare immutable documents with other documents.

Example of operations with immutable documents:
```cpp
// Create an immutable document
YYJSONObject obj = YYJSON.Parse("example.json", true);

// Reading is allowed
int value = obj.GetInt("key"); // Works fine
float fValue = obj.GetFloat("key2"); // Works fine

// Modifications will fail with clear error messages
obj.SetInt("key", 123); // Error: Cannot set value in an immutable JSON object
obj.Remove("key"); // Error: Cannot remove value from an immutable JSON object
obj.Sort(); // Error: Cannot sort an immutable JSON object

delete obj;
```

### Converting Between Mutable and Immutable
You can convert between mutable and immutable documents using deep copy:

```cpp
// Create an immutable document
YYJSONObject immutable = YYJSON.Parse("example.json", true);

// Create a mutable copy
YYJSONObject mutable = immutable.ToMutable();

// Now you can modify the mutable copy
mutable.SetInt("key", 123);

delete mutable;
delete immutable;
```

### Performance Considerations
* Use immutable documents when you only need to read JSON data
* Use mutable documents when you need to modify the JSON structure
* Immutable documents generally use less memory than mutable ones