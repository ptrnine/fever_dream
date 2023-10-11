find_package(SFML REQUIRED COMPONENTS system window graphics audio)
find_package(box2d REQUIRED)

add_executable(fever_dream app.cpp)
target_link_libraries(fever_dream sfml-window GL)
