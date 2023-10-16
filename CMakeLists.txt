cmake_minimum_required(VERSION 3.27)

project(fever_dream)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(SFML REQUIRED COMPONENTS system window graphics audio)
find_package(box2d REQUIRED)
find_package(nlohmann_json 3.11 REQUIRED)

set(LIBS sfml-window sfml-graphics GL nlohmann_json::nlohmann_json)

add_subdirectory(src)
add_subdirectory(examples)
