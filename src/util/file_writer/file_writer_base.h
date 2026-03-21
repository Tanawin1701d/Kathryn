//
// Created by tanawin on 26/1/2567.
//

#ifndef KATHRYN_FILEWRITERBASE_H
#define KATHRYN_FILEWRITERBASE_H

#include "iostream"
#include "fstream"
#include "cstring"

namespace kathryn{

    /**buffer size 256MB **/
    static int FILE_WRITE_BUF_SIZE =  1 << 28;

    class FileWriterBase{
    private:
        std::string    _fileName;
        std::ofstream* _outFile  = nullptr;
        char*          _fileBuff = nullptr;
        int            _nextBuffIdx{};

    protected:
        void open_file(const std::string& new_path);
        void close_file();

    public:
        explicit  FileWriterBase(const std::string& file_name);
        virtual  ~FileWriterBase();
        virtual void init(){};
        void add_data(const std::string& data);
        void flush();
        virtual void renew(const std::string& file_name);


    };




}

#endif //KATHRYN_FILEWRITERBASE_H
