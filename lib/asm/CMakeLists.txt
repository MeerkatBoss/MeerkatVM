add_library(libasm assembler.cpp)

target_include_directories(libasm PUBLIC
                        ${CMAKE_CURRENT_LIST_DIR})