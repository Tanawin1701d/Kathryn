cmake_minimum_required(VERSION 3.25)
project(Kathryn)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O3)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#set(CMAKE_BUILD_TYPE Debug)

#add_definitions(-DMODELCOMPILEVB) ### model compile verbose
add_definitions(-DNOTEXCEED64)

INCLUDE_DIRECTORIES( ./src )


FILE(GLOB_RECURSE SRC_ABS             RELATIVE ${CMAKE_SOURCE_DIR} "src/abstract/*.cpp")
FILE(GLOB_RECURSE SRC_APP             RELATIVE ${CMAKE_SOURCE_DIR} "src/application/*.cpp")
FILE(GLOB_RECURSE SRC_FED             RELATIVE ${CMAKE_SOURCE_DIR} "src/frontEnd/*.cpp")
FILE(GLOB_RECURSE SRC_MODEL           RELATIVE ${CMAKE_SOURCE_DIR} "src/model/*.cpp")
FILE(GLOB_RECURSE SRC_SIM             RELATIVE ${CMAKE_SOURCE_DIR} "src/sim/*.cpp")
FILE(GLOB_RECURSE SRC_GEN             RELATIVE ${CMAKE_SOURCE_DIR} "src/gen/*.cpp")
FILE(GLOB_RECURSE SRC_TEST            RELATIVE ${CMAKE_SOURCE_DIR} "src/test/*.cpp")
FILE(GLOB_RECURSE SRC_UTIL            RELATIVE ${CMAKE_SOURCE_DIR} "src/util/*.cpp")
FILE(GLOB_RECURSE SRC_EXAMPLE         RELATIVE ${CMAKE_SOURCE_DIR} "src/example/*.cpp")
FILE(GLOB_RECURSE SRC_PARAM           RELATIVE ${CMAKE_SOURCE_DIR} "src/params/*.cpp")
FILE(GLOB_RECURSE SRC_LIB             RELATIVE ${CMAKE_SOURCE_DIR} "src/lib/*.cpp")
FILE(GLOB_RECURSE SRC_MODEL_COMPILE   RELATIVE ${CMAKE_SOURCE_DIR} "src/modelCompile/proxyEventBase.cpp")


add_executable(Kathryn
        main.cpp
        #main2.cpp
        #main3.cpp
        src/kathryn.cpp
        ${SRC_ABS}
        ${SRC_APP}
        ${SRC_FED}
        ${SRC_MODEL}
        ${SRC_SIM}
        ${SRC_GEN}
        ${SRC_TEST}
        ${SRC_UTIL}
        ${SRC_EXAMPLE}
        ${SRC_PARAM}
        ${SRC_LIB}
        ${SRC_MODEL_COMPILE}
        src/sim/modelSimEngine/hwComponent/wire/wireSim.cpp
        src/sim/modelSimEngine/hwComponent/wire/wireSim.h
        src/sim/modelSimEngine/hwComponent/value/valueSim.cpp
        src/sim/modelSimEngine/hwComponent/value/valueSim.h
        src/sim/modelSimEngine/hwComponent/register/registerSim.cpp
        src/sim/modelSimEngine/hwComponent/register/registerSim.h
        src/sim/modelSimEngine/hwComponent/expression/expressionSim.cpp
        src/sim/modelSimEngine/hwComponent/expression/expressionSim.h
        src/sim/modelSimEngine/hwComponent/memBlk/memAgentSim.cpp
        src/sim/modelSimEngine/hwComponent/memBlk/memAgentSim.h
        src/sim/modelSimEngine/hwComponent/expression/nestSim.cpp
        src/sim/modelSimEngine/hwComponent/expression/nestSim.h
        src/util/fileWriter/codeWriter/cppWriter.cpp
        src/util/fileWriter/codeWriter/cppWriter.h
        src/util/fileWriter/codeWriter/codeBaseWriter.cpp
        src/util/fileWriter/codeWriter/codeBaseWriter.h
        src/sim/modelSimEngine/hwComponent/abstract/genHelper.cpp
        src/sim/modelSimEngine/hwComponent/abstract/genHelper.h
        src/sim/modelSimEngine/base/traceEvent.cpp
        src/sim/modelSimEngine/base/traceEvent.h

)
