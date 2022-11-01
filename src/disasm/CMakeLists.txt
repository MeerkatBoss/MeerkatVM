add_executable(mbdisasm
                main.cpp
                disasm_utils.cpp
                disasm_config.cpp
                disasm_flags.cpp)

target_link_libraries(mbdisasm liblogs libasm libinput libargparser)

target_include_directories(mbdisasm PRIVATE
                        ${CMAKE_CURRENT_LIST_DIR})
