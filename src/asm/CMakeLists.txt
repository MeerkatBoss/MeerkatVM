add_executable(asm main.cpp)

target_link_libraries(asm PRIVATE liblogs libasm)

target_include_directories(asm PRIVATE
                    ${CMAKE_CURRENT_LIST_DIR})