set(_examples
    particles_gravity
    particles_think
    particles_animation_keys
)

foreach(_example ${_examples})
    add_executable(${_example} ${_example}.cpp)
    target_include_directories(${_example} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${_example} ${LIBS})
endforeach()
