cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(quadratic-equation VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-D _DEBUG -ggdb3 -std=c++2a -O0 -Wall -Wextra\
    -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations\
    -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion\
    -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security\
    -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd\
    -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow\
    -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2\
    -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types\
    -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code\
    -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers\
    -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector") 

set(CMAKE_LINKER_FLAGS "${CMAKE_CXX_FLAGS}-D _DEBUG -ggdb3 -std=c++2a -O0 -Wall -Wextra\
    -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations\
    -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion\
    -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security\
    -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd\
    -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow\
    -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2\
    -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types\
    -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code\
    -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers\
    -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector") 

add_subdirectory(lib/logger)

add_subdirectory(lib/stack)

add_subdirectory(lib/text_lines)

add_subdirectory(lib/asm)

add_subdirectory(src/asm)

add_subdirectory(src/disasm)

add_subdirectory(src/vcpu)

install(TARGETS asm disasm vcpu
        RUNTIME
        CONFIGURATIONS Release)

if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
