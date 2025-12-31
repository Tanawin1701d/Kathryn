//
// Created by tanawin on 26/1/2567.
//

#ifndef KATHRYN_SIMRESWRITER_H
#define KATHRYN_SIMRESWRITER_H

#include<cassert>
#include<vector>
#include<stack>

#include "sim/logicRep/valRep.h"
#include "util/fileWriter/fileWriterBase.h"
#include "model/hwComponent/abstract/Slice.h"


namespace kathryn{

    enum VCD_SIG_TYPE{
        VST_REG     = 0,
        VST_WIRE    = 1,
        VST_INTEGER = 2,
        VST_CNT     = 3,
        VST_DUMMY   = 4
    };

    typedef unsigned long long ull;

    std::string vcdSigTypeToStr(VCD_SIG_TYPE st);

    /** for simulate data collection*/
    class VcdWriter : public FileWriterBase{


    public:
        explicit VcdWriter(std::string fileName);
        ~VcdWriter() = default;
        void addNewVar(VCD_SIG_TYPE st, std::string name, Slice sl);
        void addNewValue(const std::string& name, ull val);
        void addNewValue(const std::string& name, const std::string& value);
        void addNewTimeStamp(ull timeStamp);


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
        std::vector<TrackData> trackDatas;
    public:
        Module* _topRecMod = nullptr;

        explicit FlowWriter(const std::string& fileName);

        ~FlowWriter() = default;

        void startColFlowBlock(FlowBlockBase* fb, int ident);

        void startColModule(Module* moduleToRec, int ident);

        void startWriteSummary();

        void startWriteData();



        void init(Module* topModule);
    };




}

#endif //KATHRYN_SIMRESWRITER_H
