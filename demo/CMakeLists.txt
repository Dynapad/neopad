add_executable(neopad_demo neopad.c)
set_property(TARGET neopad_demo PROPERTY OUTPUT_NAME neopad)

target_link_libraries(neopad_demo PRIVATE neopad)