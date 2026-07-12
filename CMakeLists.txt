cmake_minimum_required(VERSION 3.20)
project(WD2ScriptHook C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W3 /WX- /MT /O2 /GS- /guard:cf-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NDEBUG NOMINMAX)
    add_link_options(/GUARD:NO /NXCOMPAT /DYNAMICBASE)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NDEBUG NOMINMAX)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ============================================================
# dinput8.dll - DInput8 proxy (entry point)
# ============================================================
add_library(dinput8 SHARED
    src/dinput8/dinput8.c
    src/dinput8/dinput8.def
)
target_include_directories(dinput8 PRIVATE src src/dinput8)
target_link_libraries(dinput8 PRIVATE dinput8.lib dxguid.lib shlwapi.lib)
set_target_properties(dinput8 PROPERTIES
    OUTPUT_NAME "dinput8"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ============================================================
# core.dll - Main ScriptHook engine
# ============================================================
add_library(core SHARED
    src/core/core.c
    src/core/hook_manager.c
    src/core/pattern_scan.c
    src/core/lua_engine.c
    src/core/game_api.c
    src/core/render.c
    src/core/script_loader.c
    src/core/crash_report.c
    src/minhook/MinHook.c
    src/lua/lua_stub.c
)
target_include_directories(core PRIVATE
    src src/core src/lua src/minhook
)
target_compile_definitions(core PRIVATE LUA_BUILDING_LUA)
target_link_libraries(core PRIVATE
    d3d11 dxgi shlwapi psapi dbghelp
    ws2_32 winmm version advapi32 shell32
)
set_target_properties(core PROPERTIES
    OUTPUT_NAME "core"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ============================================================
# skia.dll - Overlay renderer
# ============================================================
add_library(skia SHARED
    src/skia/skia_render.c
    src/skia/skia_font.c
    src/skia/skia_image.c
    src/skia/skia_surface.c
)
target_include_directories(skia PRIVATE src src/skia)
target_link_libraries(skia PRIVATE d3d11 d3dcompiler dxgi dxguid shlwapi)
set_target_properties(skia PROPERTIES
    OUTPUT_NAME "skia"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ============================================================
# crashpad_handler.exe - Crash handler
# ============================================================
add_executable(crashpad_handler WIN32
    src/crashpad/main.c
)
target_include_directories(crashpad_handler PRIVATE src src/crashpad)
target_link_libraries(crashpad_handler PRIVATE shlwapi dbghelp psapi)
set_target_properties(crashpad_handler PROPERTIES
    OUTPUT_NAME "crashpad_handler"
    SUFFIX ".exe"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
