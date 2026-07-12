add_library(skia SHARED
    skia_render.c
    skia_font.c
    skia_image.c
    skia_surface.c
)

target_include_directories(skia PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(skia PRIVATE
    d3d11.lib
    d3dcompiler.lib
    dxgi.lib
    shlwapi.lib
)

set_target_properties(skia PROPERTIES
    OUTPUT_NAME "skia"
    PDB_OUTPUT_NAME "skia"
    SUFFIX ".dll"
    PREFIX ""
)
