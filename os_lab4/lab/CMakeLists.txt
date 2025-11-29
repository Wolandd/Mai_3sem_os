cmake_minimum_required(VERSION 3.10)
project(Lab4 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Добавляем поддиректории
add_subdirectory(src/impl1)
add_subdirectory(src/impl2)
add_subdirectory(src/program1)
add_subdirectory(src/program2)

# Установка библиотек в папку lib
install(TARGETS impl1 impl2
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Установка заголовочных файлов
install(FILES include/functions.h
    DESTINATION include
)

# Установка исполняемых файлов
install(TARGETS program1 program2
    RUNTIME DESTINATION bin
)

# Тесты
enable_testing()

# Тест для impl1 (двоичная система)
add_executable(test_functions tests/test_functions.c)
target_link_libraries(test_functions impl1 m)
target_include_directories(test_functions PRIVATE include)
add_test(NAME FunctionsTest COMMAND test_functions)

# Тест для impl2 (троичная система)
add_executable(test_impl2 tests/test_impl2.c)
target_link_libraries(test_impl2 impl2)
target_include_directories(test_impl2 PRIVATE include)
add_test(NAME Impl2Test COMMAND test_impl2)

