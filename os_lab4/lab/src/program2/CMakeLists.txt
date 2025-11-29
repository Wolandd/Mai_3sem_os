add_executable(program2 main.c)

target_link_libraries(program2 dl)

target_include_directories(program2 PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Устанавливаем RPATH для корректного поиска библиотек
set_target_properties(program2 PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
    INSTALL_RPATH_USE_LINK_PATH TRUE
    BUILD_RPATH "${CMAKE_BINARY_DIR}/src/impl1:${CMAKE_BINARY_DIR}/src/impl2"
)

