cmake_minimum_required(VERSION 3.16)
project(SistemaBancario VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BANCA_COMPILAR_TESTES "Compilar testes unitarios" ON)

add_library(banca_lib INTERFACE)
target_include_directories(banca_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Biblioteca header-only com executavel principal fino
add_executable(sistema_bancario src/main.cpp)
target_link_libraries(sistema_bancario PRIVATE banca_lib)

# Garantir diretorio de dados junto ao binario por conveniencia
add_custom_command(TARGET sistema_bancario POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:sistema_bancario>/data
)

if(BANCA_COMPILAR_TESTES)
    enable_testing()
    add_executable(testes_banca tests/test_main.cpp)
    target_link_libraries(testes_banca PRIVATE banca_lib)
    add_test(NAME testes_banca COMMAND testes_banca)
endif()

install(TARGETS sistema_bancario RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(FILES README.md LICENSE DESTINATION share/doc/SistemaBancario)
