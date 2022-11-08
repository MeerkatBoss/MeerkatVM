add_executable(mbasm
                    main.cpp
                    asm_utils.cpp
                    asm_flags.cpp
                    asm_parser.cpp
                    asm_config.cpp)

target_link_libraries(mbasm PRIVATE libargparser liblogs libasm libinput)

target_include_directories(mbasm PRIVATE
                    ${CMAKE_CURRENT_LIST_DIR})

add_run_target(mbasm)