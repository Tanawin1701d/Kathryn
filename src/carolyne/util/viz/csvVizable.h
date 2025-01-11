//
// Created by tanawin on 10/1/2025.
//

#ifndef src_carolyne_util_viz_CSVVIZABLE_H
#define src_carolyne_util_viz_CSVVIZABLE_H
#include "util/viz/csv/csvFIleWriter.h"
#include "util/viz/csv/csvTable.h"

namespace kathryn::carolyne{

    ////// sample row name
    constexpr char SRN_BITWIDTH[] = "bit width";

    ///// for debugging and generating
    struct VizCsvGenable{
        virtual ~VizCsvGenable() = default;
        virtual void visual(CsvGenFile& genFile) = 0;
    };


    /////// for gen table
    struct VizCsvGenTable{ /// gen table able
        virtual ~VizCsvGenTable() = default;
        virtual CsvTable genTable() = 0;
    };



}

#endif //src_carolyne_util_viz_CSVVIZABLE_H
