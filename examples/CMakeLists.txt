set(_examples
    efx_gravity
    efx_think
    efx_keyframe_animation
    efx_json_effect
)

foreach(_example ${_examples})
    add_executable(${_example} ${_example}.cpp)
    target_include_directories(${_example} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${_example} ${LIBS})
endforeach()
