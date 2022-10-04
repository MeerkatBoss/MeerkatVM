add_library(libasm asm.cpp)

target_include_directories(libasm PUBLIC
                        ${CMAKE_CURRENT_LIST_DIR})