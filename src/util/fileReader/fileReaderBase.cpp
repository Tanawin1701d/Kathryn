//
// Created by tanawin on 4/5/2567.
//

#include <cassert>
#include "fileReaderBase.h"


namespace kathryn{


    FileReaderBase::FileReaderBase(std::string filePath):
    _fileName(filePath),
    _inputFile(filePath)
    {
        assert(_inputFile.is_open());
    }



    std::vector<std::string> FileReaderBase::readLines() {

        std::string line;
        std::vector<std::string> results;
        /*** read file */
        while(std::getline(_inputFile, line)){
            results.push_back(line);
        }

        return results;
    }

    bool FileReaderBase::isNext(){
        return _inputFile.peek() != EOF;
    }

    std::string FileReaderBase::readline(){
        std::string line;
        std::getline(_inputFile, line);
        return line;
    }




}