cmake_minimum_required(VERSION 3.28)
project(modelCompile)

set(CMAKE_CXX_STANDARD 17)


INCLUDE_DIRECTORIES( ../src )


add_executable(modelCompile
        main.cpp
        generated/proxyEvent.cpp
        ../src/util/fileWriter/fileWriterBase.cpp
        ../src/model/simIntf/base/proxyEventBase.cpp
        ../src/sim/simResWriter/simResWriter.cpp
 )
