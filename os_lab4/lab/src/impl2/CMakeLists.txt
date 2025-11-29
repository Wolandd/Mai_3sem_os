add_library(impl2 SHARED
    sin_integral.c
    translation.c
)

target_include_directories(impl2 PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(impl2 m)

set_target_properties(impl2 PROPERTIES
    OUTPUT_NAME "impl2"
    VERSION 1.0
    SOVERSION 1
    POSITION_INDEPENDENT_CODE ON
)

