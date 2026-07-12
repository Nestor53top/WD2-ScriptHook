# WD2-ScriptHook

Recreated ScriptHook for Watch Dogs 2.

## Build

```bash
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Output

- `bin/dinput8.dll` - DInput8 proxy (entry point)
- `bin/core.dll` - Main ScriptHook engine  
- `bin/skia.dll` - Overlay renderer
- `bin/crashpad_handler.exe` - Crash handler

## Install

Copy all 4 files to WD2 game directory alongside `WatchDogs2.exe`.
