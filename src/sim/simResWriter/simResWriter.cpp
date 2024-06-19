//
// Created by tanawin on 26/1/2567.
//

#include "simResWriter.h"
#include "sim/logicRep/valRep.h"


namespace kathryn{


    /**
     * vcd writer
     *
     ***/

    std::string vcdSigTypeToStr(VCD_SIG_TYPE st){
        std::string mapper[VST_CNT] = {
                "reg",
                "wire",
                "integer"
        };
        assert(st < VST_CNT);
        return mapper[st];
    }


    VcdWriter::VcdWriter(std::string fileName) : FileWriterBase(fileName) {}

    void VcdWriter::addNewVar(VCD_SIG_TYPE st, std::string name, Slice sl) {
        addData("$var " +
                vcdSigTypeToStr(st) + " " +
                std::to_string(sl.getSize()) + " " +
                name + " " +
                name + " $end\n"
                );
    }

    void VcdWriter::addNewValue(const std::string& name, ull val) {
        std::string preRet ;
        std::bitset<bitSizeOfUll> binaryRepresentation(val);
        preRet += binaryRepresentation.to_string();
        addData(
                "b" + preRet +
                " " + name + "\n"
                );
    }


    void VcdWriter::addNewTimeStamp(ull timeStamp){
        addData("#" + std::to_string(timeStamp) + "\n");
    }

    ////////////////////////////////////// for flow collector

    FlowWriter::FlowWriter(std::string fileName)
        : FileWriterBase(fileName){

        init();

    }

    FlowWriter::~FlowWriter(){
        startWriteData();
    }

    void FlowWriter::startWriteData(){
        assert(startEle != nullptr);

        std::stack<FlowColEle*> dfsSt;
        addData(startEle->getPrintStr());
        dfsSt.push(startEle);

        /** do depth first search*/
        while (!dfsSt.empty()){
            ////////// get element
            FlowColEle* flowColEle = dfsSt.top();
            ////////// case thisEle
            if (flowColEle->isAllSubElePrinted()){
                dfsSt.pop();
            }else{
                /**we must print next*/
                FlowColEle* nextFlowEle = flowColEle->getNextPrintSubEle();
                flowColEle->iteratePrintIdx();
                addData(nextFlowEle->getPrintStr());
                dfsSt.push(nextFlowEle);
            }
        }
    }

    void FlowWriter::init(){
        delete startEle;
        startEle = new FlowColEle({"MAIN_FLOW_COLLECTOR",0});
    }

}