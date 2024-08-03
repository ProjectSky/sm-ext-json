# SourceMod JSON Extension

## What is this?
This is a [SourceMod](http://www.sourcemod.net/) extension that provides some methods for manipulating JSON data

## Features
* Relies on [YYJSON](https://github.com/ibireme/yyjson) which A high performance JSON library written in ANSI C
* Supports querying and modifying using [JSON Pointer](https://datatracker.ietf.org/doc/html/rfc6901)
* Supports decoding and encoding (also with pretty format)
* Supports x64

## How to build this?
``` bash
clone project
mkdir build && cd build
python ../configure.py --enable-optimize --symbol-files --sm-path=YOU_SOURCEMOD_PATH --targets=x64,x86
ambuild
```

## Native
* [yyjson.inc](https://github.com/ProjectSky/sm-ext-yyjson/blob/main/scripting/include/yyjson.inc)

# Binary files
* [GitHub Actions](https://github.com/ProjectSky/sm-ext-yyjson/actions)

# NOTES
* Not fully tested, some function may not work

# Example
``` cpp
// Create JSONObject
YYJSONObject hJSONObject = new YYJSONObject();
hJSONObject.SetInt("int", 1);
hJSONObject.SetInt64("int64", "9223372036854775800");
hJSONObject.SetFloat("float", 2.0);
hJSONObject.SetBool("bool", true);
hJSONObject.SetString("str", "Hello World");
hJSONObject.SetNull("null");
delete hJSONObject;
/*
{
  "int": 1,
  "int64": 9223372036854775800,
  "float": 2.0,
  "bool": true,
  "str": "Hello World",
  "null": null
}
*/

// Create JSONArray
YYJSONArray hJSONArray = new YYJSONArray();
hJSONArray.PushInt(1);
hJSONArray.PushInt64("9223372036854775800");
hJSONArray.PushFloat(2.0);
hJSONArray.PushBool(true);
hJSONArray.PushString("Hello World");
hJSONArray.PushNull();
delete hJSONArray;
/*
[
  1,
  9223372036854775800,
  2.0,
  true,
  "Hello World",
  null
]
*/

// JSON POINTER: Create JSONObject
YYJSONObject hJSONObject = new YYJSONObject();
hJSONObject.PtrSetInt("/a/b/c", 1);
delete hJSONObject;
/*
{
  "a": {
    "b": {
      "c": 1
    }
  }
}
*/

// JSON POINTER: Query data
/*
{
  "bool": true,
  "float": 1.234,
  "int": 1234,
  "int64": 9223372036854775807,
  "null": null,
  "string": "hello world",
  "arr": [
    false,
    1.2344,
    1234,
    9223372036854775807,
    null,
    "Hello World"
  ]
}
*/
YYJSONObject hPtrTest = YYJSON.Parse("example.json", true);
hPtrTest.PtrGetInt("/int"); // 1234
hPtrTest.PtrGetFloat("/arr/1"); // 1.2344
delete hPtrTest;

// Iteration: Objects
char key[64];
YYJSON val;
for (int i = 0; i < hPtrTest.Size; i++)
{
  hPtrTest.GetKeyName(i, key, sizeof(key));
  PrintToServer("key: %s", key);
  val = hPtrTest.GetValueAt(i);
  // do something
  delete val;
}

// Iteration: Arrays
YYJSONArray arr = hPtrTest.PtrGet("/arr");
YYJSON val;
for (int i = 0; i < arr.Length; i++)
{
  val = arr.Get(i);
  // do something
  delete val;
}
delete arr;
```