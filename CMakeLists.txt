cmake_minimum_required(VERSION 3.27)

project(fever_dream)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(SFML REQUIRED COMPONENTS system window graphics audio)
find_package(box2d REQUIRED)
find_package(nlohmann_json 3.11 REQUIRED)
find_package(imgui REQUIRED)
find_package(ImGui-SFML REQUIRED)

set(LIBS sfml-window sfml-graphics GL nlohmann_json::nlohmann_json ImGui-SFML::ImGui-SFML)

option(ENABLE_ASAN "Enable address sanitizer" OFF)
if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
endif()

add_subdirectory(src)
add_subdirectory(examples)
