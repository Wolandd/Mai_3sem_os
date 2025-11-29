add_executable(program1 main.c)

target_link_libraries(program1 impl1)

target_include_directories(program1 PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Устанавливаем RPATH для корректного поиска библиотек
set_target_properties(program1 PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

