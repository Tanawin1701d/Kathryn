cmake_minimum_required(VERSION 3.26)
project(Kathryn)

set(CMAKE_CXX_STANDARD 17)


INCLUDE_DIRECTORIES( ./src )


FILE(GLOB_RECURSE SRC_ABS   RELATIVE ${CMAKE_SOURCE_DIR} "src/abstract/*.cpp")
FILE(GLOB_RECURSE SRC_APP   RELATIVE ${CMAKE_SOURCE_DIR} "src/application/*.cpp")
FILE(GLOB_RECURSE SRC_FED   RELATIVE ${CMAKE_SOURCE_DIR} "src/frontEnd/*.cpp")
FILE(GLOB_RECURSE SRC_MODEL RELATIVE ${CMAKE_SOURCE_DIR} "src/model/*.cpp")
FILE(GLOB_RECURSE SRC_SIM   RELATIVE ${CMAKE_SOURCE_DIR} "src/sim/*.cpp")
FILE(GLOB_RECURSE SRC_TEST  RELATIVE ${CMAKE_SOURCE_DIR} "src/test/*.cpp")
FILE(GLOB_RECURSE SRC_UTIL  RELATIVE ${CMAKE_SOURCE_DIR} "src/util/*.cpp")


add_executable(Kathryn
        main.cpp
        src/kathryn.cpp
        ${SRC_ABS}
        ${SRC_APP}
        ${SRC_FED}
        ${SRC_MODEL}
        ${SRC_SIM}
        ${SRC_TEST}
        ${SRC_UTIL}
        src/example/uart/rxUart.h
        src/example/uart/txUart.h
        src/example/riscv/subSystem/storageMgm.h
        src/example/riscv/subSystem/regMgm.h
        src/example/riscv/core/core.h
        src/example/riscv/core/fetch.h
        src/example/riscv/parameter.h
        src/example/riscv/core/decode.h
        src/example/riscv/element.h
        src/example/numberic/extend.h

)
