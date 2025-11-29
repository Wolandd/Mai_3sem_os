add_library(impl1 SHARED
    sin_integral.c
    translation.c
)

target_include_directories(impl1 PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(impl1 m)

set_target_properties(impl1 PROPERTIES
    OUTPUT_NAME "impl1"
    VERSION 1.0
    SOVERSION 1
    POSITION_INDEPENDENT_CODE ON
)

