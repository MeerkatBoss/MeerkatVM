add_executable(vcpu main.cpp vcpu_utils.cpp)

target_link_libraries(vcpu PRIVATE libargparser libasm liblogs libstack)

target_include_directories(vcpu PRIVATE
                        ${CMAKE_CURRENT_LIST_DIR})