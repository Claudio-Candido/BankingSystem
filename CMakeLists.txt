cmake_minimum_required(VERSION 3.16)
project(BankingSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BANKING_BUILD_TESTS "Build unit tests" ON)

add_library(banking_lib INTERFACE)
target_include_directories(banking_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Header-only library with a thin main executable
add_executable(banking_system src/main.cpp)
target_link_libraries(banking_system PRIVATE banking_lib)

# Ensure data directory exists next to the binary for convenience
add_custom_command(TARGET banking_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:banking_system>/data
)

if(BANKING_BUILD_TESTS)
    enable_testing()
    add_executable(banking_tests tests/test_main.cpp)
    target_link_libraries(banking_tests PRIVATE banking_lib)
    add_test(NAME banking_tests COMMAND banking_tests)
endif()

install(TARGETS banking_system RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(FILES README.md LICENSE DESTINATION share/doc/BankingSystem)
