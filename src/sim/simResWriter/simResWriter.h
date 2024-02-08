//
// Created by tanawin on 26/1/2567.
//

#ifndef KATHRYN_SIMRESWRITER_H
#define KATHRYN_SIMRESWRITER_H

#include<cassert>
#include <vector>
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

    class ValRep;
    class VcdWriter : public FileWriterBase{


    public:
        explicit VcdWriter(std::string fileName);
        ~VcdWriter() = default;
        void addNewVar(VCD_SIG_TYPE st, std::string name, Slice sl);
        void addNewValue(const std::string& name, ValRep& val);
        void addNewTimeStamp(ull timeStamp);


    };

    struct flowColEle{
        std::string localName;
        int freq = 0;
        std::vector<flowColEle*> subEle;

        flowColEle* populateSubEle(){
            subEle.push_back(new flowColEle);
            return *subEle.rbegin();
        }

    };

    class FlowCollector : public FileWriterBase{
    private:
        flowColEle* startEle = nullptr;

    public:
        FlowCollector(std::string fileName);

        flowColEle* getstartEle() {
            assert(startEle != nullptr);
            return startEle;
        }

        void startWriteData();
    };




}

#endif //KATHRYN_SIMRESWRITER_H
