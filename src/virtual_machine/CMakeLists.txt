add_executable(mbvm main.cpp vm_utils.cpp)

target_link_libraries(mbvm PRIVATE libargparser libasm liblogs libstack libinput)

target_include_directories(mbvm PRIVATE
                        ${CMAKE_CURRENT_LIST_DIR})

add_run_target(mbvm)