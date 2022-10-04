add_library(libutils utils.cpp)

target_include_directories(libutils PUBLIC
                        ${CMAKE_CURRENT_LIST_DIR})