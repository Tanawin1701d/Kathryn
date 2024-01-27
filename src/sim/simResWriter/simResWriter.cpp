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
                std::to_string(sl.getSize()) +
                name + " " +
                name + " $end\n"
                );
    }

    void VcdWriter::addNewValue(const std::string& name, ValRep &val) {
        addData(
                "b" + val.getBiStr() +
                " " + name + "\n"
                );
    }

    void VcdWriter::addNewTimeStamp(ull timeStamp){
        addData("#" + std::to_string(timeStamp) + "\n");
    }



}