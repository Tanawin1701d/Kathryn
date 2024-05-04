//
// Created by tanawin on 26/1/2567.
//

#include "fileWriterBase.h"


namespace kathryn{


    FileWriterBase::FileWriterBase(std::string fileName):
    _fileName(fileName),
    _nextBuffIdx(0)
    {
        _outFile  = new std::ofstream(fileName);
        _fileBuff = new char[FILE_WRITE_BUF_SIZE];

            if (!_outFile->is_open()){
                throw std::runtime_error("file open fail @ des :" + fileName);
            }

    }

    FileWriterBase::~FileWriterBase() {
        flush();
        delete   _outFile;
        delete[] _fileBuff;

    }

    void FileWriterBase::addData(const std::string& data) {

        /**if buffer is full -> flush it*/
        if ( (_nextBuffIdx + data.size()) > FILE_WRITE_BUF_SIZE ){
            flush();
        }
        /**copy data to array*/
        if (!data.empty()){
            std::strcpy(_fileBuff + _nextBuffIdx, data.c_str());
            _nextBuffIdx += (int)data.size();
        }



    }

    void FileWriterBase::flush() {
        _outFile->write(_fileBuff, _nextBuffIdx);
        _nextBuffIdx = 0;
    }
}