//
// Created by tanawin on 16/1/26.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_RESULTWRITER_H
#define EXAMPLE_O3_SIMCOMPARE_RESULTWRITER_H
#include <vector>

#include "sim/simResWriter/simResWriter.h"
#include "util/fileWriter/fileWriterBase.h"

namespace kathryn::o3{

    class ResultWriter: public FileWriterBase{
    public:
        std::vector<uint32_t> _results;
        ull _cycleCnt;

        ResultWriter(const std::string& fileName);
        virtual ~ResultWriter();

        void fillResult(uint32_t value);
        void fillCycleCnt(ull cycleCnt);

        void finalizeTheWriteData();

        void renew(const std::string& fileName) override;




    };

}

#endif //EXAMPLE_O3_SIMCOMPARE_RESULTWRITER_H