add_executable(disasm main.cpp disasm_utils.cpp)

target_link_libraries(disasm liblogs libasm libinput)

target_include_directories(disasm PRIVATE
                        ${CMAKE_CURRENT_LIST_DIR})
