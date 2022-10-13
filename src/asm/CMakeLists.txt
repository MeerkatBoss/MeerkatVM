add_executable(asm main.cpp asm_utils.cpp asm_flags.cpp)

target_link_libraries(asm PRIVATE libargparser liblogs libasm libinput)

target_include_directories(asm PRIVATE
                    ${CMAKE_CURRENT_LIST_DIR})