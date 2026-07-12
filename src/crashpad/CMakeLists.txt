add_executable(crashpad_handler WIN32 main.c)

target_include_directories(crashpad_handler PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(crashpad_handler PRIVATE
    shlwapi.lib
    dbghelp.lib
)

set_target_properties(crashpad_handler PROPERTIES
    OUTPUT_NAME "crashpad_handler"
    PDB_OUTPUT_NAME "crashpad_handler"
    SUFFIX ".exe"
    PREFIX ""
)
