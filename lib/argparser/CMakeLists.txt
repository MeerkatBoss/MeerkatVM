add_library(libargparser argparser.cpp)

target_link_libraries(libargparser PRIVATE liblogs)

target_include_directories(libargparser PUBLIC
                    ${CMAKE_CURRENT_LIST_DIR})