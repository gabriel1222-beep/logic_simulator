cmake_minimum_required(VERSION 3.20)

project(MyProject LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "-O0 -std=c++20 -fsanitize=address -g")

set(SRC_PATH src/main.cpp)
set(INCLUDE_PATH include/)
set(BIN_PATH app)

find_package(fmt CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

add_executable(${BIN_PATH} ${SRC_PATH})

target_include_directories(${BIN_PATH} PUBLIC ${INCLUDE_PATH})
target_link_libraries(${BIN_PATH} PRIVATE
  fmt::fmt
  $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
  $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>,
  $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>,
  magic_enum::magic_enum
)
