cmake_minimum_required(VERSION 3.10)
project(os_lab1)

# Установка стандарта C
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Компиляция дочернего процесса child
add_executable(child child.c)

# Компиляция родительского процесса parent
add_executable(parent parent.c)

# Установка выходных файлов в директорию build
set_target_properties(child PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(parent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)


