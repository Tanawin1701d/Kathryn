//
// Created by tanawin on 26/1/2567.
//

#include "simResWriter.h"

#include <model/hwComponent/module/module.h>

#include "sim/modelSimEngine/flowBlock/flowBaseSim.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "util/str/strUtil.h"


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

        addNewValue(name, preRet);
    }

    void VcdWriter::addNewValue(const std::string& name, const std::string& value){
        ///std::cout << "addDayta"  << name << "    " << value << std::endl;
        addData(
            "b" + value +
            " " + name  + "\n"
        );

    }

    void VcdWriter::addNewTimeStamp(ull timeStamp){
        addData("#" + std::to_string(timeStamp) + "\n");
    }

    ////////////////////////////////////// for flow collector

    FlowWriter::FlowWriter(const std::string& fileName)
        : FileWriterBase(fileName){}

    void FlowWriter::startColFlowBlock(FlowBlockBase* fb, int ident){

        /////// this cycle ident
        std::string         indentStr = genConString(' ', ident);
        std::string         showName  = fb->getGlobalName();
        FlowBaseSimEngine*  fbse      = fb->getSimEngine();
        ValRepBase&         repBase   = fbse->getProxyRep();
        ull                 cycle     = repBase.getVal();

        std::string         trackName; ///track name is user optional define trackerName
        if (fb->isZepTrackNameSet()){
            trackDatas.push_back({fb->getZepTrackName(), cycle});
            trackName = " >>> " + fb->getZepTrackName();
        }


        addData(indentStr + showName + "    " +
            std::to_string(cycle) + trackName + "\n");
        /////// sub block Ident
        for (FlowBlockBase* subBlock: fb->getSubBlocks()){
            assert(subBlock != nullptr);
            startColFlowBlock(subBlock, ident + SUBFLOW_IDENT);
        }
        if (!fb->getConBlocks().empty()){
            std::string conIdentStr = genConString(' ', ident + CONFLOW_IDENT);
            addData(conIdentStr + "-conBlock-\n");
            for (FlowBlockBase* conBlock: fb->getConBlocks()){
                assert(conBlock != nullptr);
                startColFlowBlock(conBlock, ident + CONFLOW_IDENT);
            }
        }
    }

    void FlowWriter::startColModule(Module* moduleToRec, int ident){
        std::string indentStr = genConString(' ',ident);
        std::string showName = "Module " + moduleToRec->getGlobalName();
        addData(indentStr + showName + "\n");
        for (FlowBlockBase* fb: moduleToRec->getFlowBlocks()){
            startColFlowBlock(fb, ident + SUBFLOW_IDENT);
        }
        if (!moduleToRec->getUserSubModules().empty()){
            std::string subIndentStr = genConString(' ', ident + SUBMOD_IDENT);
            addData(subIndentStr + "-subMod-\n");
            for (Module* subModule: moduleToRec->getUserSubModules()){
                startColModule(subModule, ident + SUBMOD_IDENT);
            }
        }
    }

    void FlowWriter::startWriteSummary(){
        int maxLength = 0;
        ////find max length to make the match column
        for (const auto&[key, value]: trackDatas){
            maxLength = std::max(static_cast<int>(key.size()), maxLength);
        }

        for (const auto&[key, value]: trackDatas){
            addData(key + genConString(' ', maxLength+1-key.size())
                        + " : " + std::to_string(value) + "\n");
        }
    }

    void FlowWriter::startWriteData(){
        startColModule(_topRecMod, 0);
        addData("\n\n\n\n\n ------------- User Defined Summary -------------\n");
        startWriteSummary();
    }

    void FlowWriter::init(Module* topModule){
        assert(topModule != nullptr);
        _topRecMod = topModule;
    }

}