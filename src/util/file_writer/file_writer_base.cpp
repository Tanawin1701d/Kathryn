//
// Created by tanawin on 26/1/2567.
//

#include "file_writer_base.h"

#include "cassert"


namespace kathryn{


    FileWriterBase::FileWriterBase(const std::string& file_name){
        open_file(file_name);
        _fileBuff = new char[FILE_WRITE_BUF_SIZE];
        if (!_outFile->is_open()){
            int x = 0;
            throw std::runtime_error("file open fail @ des :" + file_name);
        }
    }

    FileWriterBase::~FileWriterBase() {
        close_file();
        delete[] _fileBuff;
    }

    void FileWriterBase::open_file(const std::string& new_path){
        assert(_outFile == nullptr);
        _fileName = new_path;
        _outFile  = new std::ofstream(_fileName);
        _nextBuffIdx = 0;
    }

    void FileWriterBase::close_file(){
        assert(_outFile != nullptr);
        flush();
        _outFile->close();
        _outFile = nullptr;
        _nextBuffIdx = 0;
    }

    void FileWriterBase::add_data(const std::string& data) {

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

    void FileWriterBase::renew(const std::string& file_name){
        close_file();
        open_file(file_name);
        init();
    }
}
