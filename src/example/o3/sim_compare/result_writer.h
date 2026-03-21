//
// Created by tanawin on 16/1/26.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_RESULTWRITER_H
#define EXAMPLE_O3_SIMCOMPARE_RESULTWRITER_H
#include "vector"

#include "sim/sim_res_writer/sim_res_writer.h"
#include "util/file_writer/file_writer_base.h"

namespace kathryn::o3{

    class ResultWriter: public FileWriterBase{
    public:
        std::vector<uint32_t> _results;
        ull _cycleCnt;

        ResultWriter(const std::string& file_name);
        virtual ~ResultWriter();

        void fill_result(uint32_t value);
        void fill_cycle_cnt(ull cycle_cnt);

        void finalize_the_write_data();

        void renew(const std::string& file_name) override;




    };

}

#endif //EXAMPLE_O3_SIMCOMPARE_RESULTWRITER_H