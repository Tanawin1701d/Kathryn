//
// Created by tanawin on 16/1/26.
//

#include "resultWriter.h"

namespace kathryn::o3{


    ResultWriter::ResultWriter(const std::string& fileName) :
    FileWriterBase(fileName),
    _cycleCnt(0){
    }

    ResultWriter::~ResultWriter(){
        //finalizeTheWriteData();
    }

    void ResultWriter::fillResult(uint32_t value){
        _results.push_back(value);
    }

    void ResultWriter::fillCycleCnt(ull cycleCnt){
        _cycleCnt = cycleCnt;
    }

    void ResultWriter::finalizeTheWriteData(){

        addData(std::to_string(_cycleCnt) + "\n");
        addData("----- output -----\n");
        for (auto& result: _results){
            char resultChar = static_cast<char>(result);
            std::string resultStr {resultChar};
            addData(resultStr);
        }
        _cycleCnt = 0;
        _results.clear();
    }

    void ResultWriter::renew(const std::string& fileName){

        finalizeTheWriteData();
        FileWriterBase::renew(fileName);

    }


}
