# SourceMod JSON Extension

## What is this?
This is a [SourceMod](http://www.sourcemod.net/) extension that provides some methods for manipulating JSON data

## Features
* Supports decoding and encoding (also with pretty format)
* Relies on [Parson](https://github.com/kgabis/parson) which is lighweight and simple JSON library written in C
* Supports dot notation (Values can be accessed by typing objectA.objectB.value)
* Allows us to iterate through arrays and objects

## How to build this?
``` bash
clone project
mkdir build && cd build
python ../configure.py --enable-optimize --symbol-files --sm-path=YOU_SOURCEMOD_PATH
ambuild
```

## Native
* see [json.inc](https://github.com/ProjectSky/sm-ext-json/blob/main/scripting/include/json.inc)

# Binary files
* https://github.com/ProjectSky/sm-ext-json/actions

## TODO
- [x] windows support
- [x] Int64 support
- [ ] example script

## Known issues
* This is my first porting an extension, so it might have a lot of issues
* Some functions will crash the server if passed incorrect data
* incompatible with rip-ext due to naming conflict

## Link
* https://github.com/alliedmodders/sourcemod/issues/1468
* https://github.com/ErikMinekus/sm-ripext
* https://github.com/alliedmodders/amxmodx/pull/379