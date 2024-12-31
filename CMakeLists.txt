cmake_minimum_required(VERSION 3.22)
project(Kathryn)

set(CMAKE_CXX_STANDARD 17)

#add_compile_options(-O3)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#set(CMAKE_BUILD_TYPE Debug)

#add_definitions(-DMODELCOMPILEVB) ### model compile verbose
add_definitions(-DNOTEXCEED64)
add_compile_definitions(KATHRYN_PROJ_FOLD_PATH=${CMAKE_SOURCE_DIR})

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
        src/carolyne/arch/base/util/sliceMatcher.h
        src/carolyne/arch/base/isa/uop/opr_base.h
        src/carolyne/arch/base/isa/uop/uop_base.h
        src/carolyne/arch/base/isa/mop/mop_base.h
        src/carolyne/arch/base/isa/repo/repo.h
        src/carolyne/arch/base/isa/regFile/archRegFile.h
        src/carolyne/arch/base/util/regFileBase.h
        src/carolyne/arch/base/util/rowMeta.h
        src/carolyne/arch/base/util/tableMeta.h
        src/carolyne/arch/base/march/pRegFile/physicalRegFile_base.h
        src/carolyne/arch/base/march/pRegFile/physicalRegFile_base.h
        src/carolyne/arch/base/march/fetchUnit/fetchMetaBase.h
        src/carolyne/arch/base/march/alloc/allocUnit.h
        src/carolyne/arch/base/march/robUnit/robMetaBase.h
        src/carolyne/arch/base/march/execUnit/execMetaBase.h
        src/carolyne/arch/base/march/repo/repo.h
        src/carolyne/arch/base/isa/uop/opr_lrf.h
        src/carolyne/arch/base/isa/uop/opr_li.h
        src/carolyne/arch/base/isa/uop/opr_srf.h
        #blinkSample.cpp
        #src/params/prjParam.h
)
