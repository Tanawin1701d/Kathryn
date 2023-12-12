//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_LOGGER_H
#define KATHRYN_LOGGER_H


#include<string>

namespace kathryn{



    enum LOG_TYPE{

        LOG_FLOWBLOCK = 0,
        LOG_COUNT = 1

    };

    struct logMeta{
        unsigned long long id;
        std::string debugMsg;
    };


    extern logMeta logStorage[LOG_COUNT];

    void log(const char* format, ...);




}

#endif //KATHRYN_LOGGER_H
