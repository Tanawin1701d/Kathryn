//
// Created by tanawin on 12/12/2566.
//
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include "logger.h"


namespace kathryn{

    logMeta logStorage[LOG_COUNT];

    void log(const char* format, ...){
        constexpr size_t bufferSize = 1024;
        char buffer[bufferSize];
        /**start args*/
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, bufferSize, format, args);
        va_end(args);
        std::string result(buffer);
        std::cout << result;
    }





}