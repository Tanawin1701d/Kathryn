//
// Created by tanawin on 16/1/26.
//

#include "result_writer.h"

namespace kathryn::o3{


    ResultWriter::ResultWriter(const std::string& file_name) :
    FileWriterBase(file_name),
    _cycleCnt(0){
    }

    ResultWriter::~ResultWriter(){
        //finalize_the_write_data();
    }

    void ResultWriter::fill_result(uint32_t value){
        _results.push_back(value);
    }

    void ResultWriter::fill_cycle_cnt(ull cycle_cnt){
        _cycleCnt = cycle_cnt;
    }

    void ResultWriter::finalize_the_write_data(){

        add_data(std::to_string(_cycleCnt) + "\n");
        add_data("----- output -----\n");
        for (auto& result: _results){
            char result_char = static_cast<char>(result);
            std::string result_str {result_char};
            add_data(result_str);
        }
        _cycleCnt = 0;
        _results.clear();
    }

    void ResultWriter::renew(const std::string& file_name){

        finalize_the_write_data();
        FileWriterBase::renew(file_name);

    }


}
