//
// Created by tanawin on 26/1/2567.
//

#ifndef KATHRYN_SIMRESWRITER_H
#define KATHRYN_SIMRESWRITER_H

#include "cassert"
#include "vector"
#include "stack"

#include "sim/logic_rep/val_rep.h"
#include "util/file_writer/file_writer_base.h"
#include "model/hw_component/abstract/slice.h"


namespace kathryn{

    enum VCD_SIG_TYPE{
        VST_REG     = 0,
        VST_WIRE    = 1,
        VST_INTEGER = 2,
        VST_CNT     = 3,
        VST_DUMMY   = 4
    };

    typedef unsigned long long ull;

    std::string vcd_sig_type_to_str(VCD_SIG_TYPE st);

    /** for simulate data collection*/
    class VcdWriter : public FileWriterBase{


    public:
        explicit VcdWriter(std::string file_name);
        ~VcdWriter() = default;
        void add_new_var(VCD_SIG_TYPE st, std::string name, Slice sl);
        void add_new_value(const std::string& name, ull val);
        void add_new_value(const std::string& name, const std::string& value);
        void add_new_time_stamp(ull time_stamp);


    };



    class Module;
    class FlowBlockBase;

    constexpr int CONFLOW_IDENT = 3;
    constexpr int SUBFLOW_IDENT = 6;
    constexpr int SUBMOD_IDENT  = 6;


    class FlowWriter : public FileWriterBase{
    protected:
        struct TrackData{
            std::string name;
            ull         values;
        };
        std::vector<TrackData> track_datas;
    public:
        Module* _topRecMod = nullptr;

        explicit FlowWriter(const std::string& file_name);

        ~FlowWriter() = default;

        void start_col_flow_block(FlowBlockBase* fb, int ident);

        void start_col_module(Module* module_to_rec, int ident);

        void start_write_summary();

        void start_write_data();



        void init(Module* top_module);
    };




}

#endif //KATHRYN_SIMRESWRITER_H
