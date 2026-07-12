add_library(core SHARED
    core.c
    lua_engine.c
    game_api.c
    minhook.c
    hook_manager.c
    crash_report.c
    render.c
    script_loader.c
)

target_include_directories(core PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lua
)

target_link_libraries(core PRIVATE
    shlwapi.lib
    winmm.lib
    version.lib
    advapi32.lib
    shell32.lib
    ws2_32.lib
)

set_target_properties(core PROPERTIES
    OUTPUT_NAME "core"
    PDB_OUTPUT_NAME "core"
    SUFFIX ".dll"
    PREFIX ""
)
