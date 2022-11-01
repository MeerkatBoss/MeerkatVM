add_executable(mbdisasm main.cpp disasm_utils.cpp)

target_link_libraries(mbdisasm liblogs libasm libinput)

target_include_directories(mbdisasm PRIVATE
                        ${CMAKE_CURRENT_LIST_DIR})
