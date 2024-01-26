//
// Created by tanawin on 26/1/2567.
//

#ifndef KATHRYN_FILEWRITERBASE_H
#define KATHRYN_FILEWRITERBASE_H

#include <iostream>
#include<fstream>
#include <cstring>

namespace kathryn{

    /**buffer size 256MB **/
    static int FILE_WRITE_BUF_SIZE =  1 << 28;

    class FileWriterBase{
    private:
        std::string    _fileName;
        std::ofstream* _outFile  = nullptr;
        char*          _fileBuff = nullptr;
        int            _nextBuffIdx;

    public:
        explicit FileWriterBase(std::string fileName);
        virtual ~FileWriterBase();
        void addData(const std::string& data);
        void flush();
    };




}

#endif //KATHRYN_FILEWRITERBASE_H
