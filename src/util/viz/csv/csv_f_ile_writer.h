//
// Created by tanawin on 10/1/2025.
//

#ifndef src_util_viz_csv_CSVFILEWRITER_H
#define src_util_viz_csv_CSVFILEWRITER_H

#include "util/file_writer/file_writer_base.h"
#include "util/str/str_util.h"
#include "csv_table.h"


namespace kathryn{

    class CsvGenFile: FileWriterBase{
    public:
        explicit CsvGenFile(const std::string& path):
        FileWriterBase(path){}

        void add_data(const CsvTable& table){
            FileWriterBase::add_data(table.gen_str());
        }

        void add_row_data(const std::string& row_name){
            FileWriterBase::add_data(row_name);
        }

        void add_dummy_row_data(int amt_line){
            assert(amt_line > 0);
            FileWriterBase::
            add_data(gen_con_string('\n',amt_line));
        }

    };


}

#endif //src_util_viz_csv_CSVFILEWRITER_H
