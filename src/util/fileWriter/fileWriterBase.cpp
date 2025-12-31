//
// Created by tanawin on 26/1/2567.
//

#include "fileWriterBase.h"

#include <cassert>


namespace kathryn{


    FileWriterBase::FileWriterBase(const std::string& fileName){
        openFile(fileName);
        _fileBuff = new char[FILE_WRITE_BUF_SIZE];
        if (!_outFile->is_open()){
            int x = 0;
            throw std::runtime_error("file open fail @ des :" + fileName);
        }
    }

    FileWriterBase::~FileWriterBase() {
        closeFile();
        delete[] _fileBuff;
    }

    void FileWriterBase::openFile(const std::string& newPath){
        assert(_outFile == nullptr);
        _fileName = newPath;
        _outFile  = new std::ofstream(_fileName);
        _nextBuffIdx = 0;
    }

    void FileWriterBase::closeFile(){
        assert(_outFile != nullptr);
        flush();
        _outFile->close();
        _outFile = nullptr;
        _nextBuffIdx = 0;
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

    void FileWriterBase::renew(const std::string& fileName){
        closeFile();
        openFile(fileName);
        init();
    }
}
