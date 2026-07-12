add_library(dinput8 SHARED dinput8.c dinput8.def)

target_include_directories(dinput8 PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(dinput8 PRIVATE
    dinput8.lib
    dxguid.lib
    shlwapi.lib
)

set_target_properties(dinput8 PROPERTIES
    OUTPUT_NAME "dinput8"
    PDB_OUTPUT_NAME "dinput8"
    SUFFIX ".dll"
    PREFIX ""
)
