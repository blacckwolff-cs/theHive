cmake_minimum_required(VERSION 3.15)
project(fireteam_ai)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# Collect all source files in /src
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Create the executable FIRST
add_executable(fireteam_ai ${SRC_FILES})

# Include directories
target_include_directories(fireteam_ai PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bots
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fireteam
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hive
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json/single_include
)

find_package(ZeroMQ REQUIRED)
target_link_libraries(fireteam_ai PRIVATE libzmq)

