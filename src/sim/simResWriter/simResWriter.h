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


    /***/

    struct FlowColEle{
        std::string localName;
        int freq = 0;
        std::vector<FlowColEle*> subEle;
        /** for printing and traversing*/
        int nextPrintedId = 0;
        int curprintIdent = 0;

        FlowColEle* populateSubEle(){
            subEle.push_back(new FlowColEle);
            return *subEle.rbegin();
        }

        bool isAllSubElePrinted() const{
            return nextPrintedId == subEle.size();
        }
        FlowColEle* getNextPrintSubEle(){
            subEle[nextPrintedId]->curprintIdent = curprintIdent + 4;
            return subEle[nextPrintedId];
        }
        void iteratePrintIdx(){
            nextPrintedId++;
        }

        std::string getPrintStr() const{
            std::string ret;
            for (int i = 0; i < curprintIdent; i++){
                ret += " ";
            }
            ret += localName;
            ret += "      ";
            ret += std::to_string(freq);
            ret += "\n";
            return ret;
        }



    };

    class FlowWriter : public FileWriterBase{
    private:
        FlowColEle* startEle = nullptr;

    public:
        explicit FlowWriter(std::string fileName);

        ~FlowWriter();

        FlowColEle* getstartEle() {
            assert(startEle != nullptr);
            return startEle;
        }

        void startWriteData();

        void init() override;
    };




}

#endif //KATHRYN_SIMRESWRITER_H
