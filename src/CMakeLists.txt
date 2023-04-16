file(GLOB SRC_LIST *.cpp)
add_executable(untitled ${SRC_LIST})
target_include_directories(untitled PUBLIC ${OUTPUT_INCLUDE})
install(TARGETS untitled DESTINATION bin)

add_custom_command(
    TARGET untitled
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install
    )