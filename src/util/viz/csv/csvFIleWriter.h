//
// Created by tanawin on 10/1/2025.
//

#ifndef src_util_viz_csv_CSVFILEWRITER_H
#define src_util_viz_csv_CSVFILEWRITER_H

#include "util/fileWriter/fileWriterBase.h"
#include "util/str/strUtil.h"
#include "csvTable.h"


namespace kathryn{

    class CsvGenFile: FileWriterBase{
    public:
        explicit CsvGenFile(const std::string& path):
        FileWriterBase(path){}

        void addData(const CsvTable& table){
            FileWriterBase::addData(table.genStr());
        }

        void addRowData(const std::string& rowName){
            FileWriterBase::addData(rowName);
        }

        void addDummyRowData(int amtLine){
            assert(amtLine > 0);
            FileWriterBase::
            addData(genConString('\n',amtLine));
        }

    };


}

#endif //src_util_viz_csv_CSVFILEWRITER_H
