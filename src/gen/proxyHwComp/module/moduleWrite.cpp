//
// Created by tanawin on 1/7/2024.
//
#include "model/hwComponent/module/module.h"

#include "gen/controller/genController.h"


namespace kathryn{


    void ModuleGen::startWriteFile(FileWriterBase* fileWriter,
                                   GenStructure* genStructure,
                                   bool isExplicitMod,
                                   const std::string& explicitModName){
        assert(genStructure != nullptr);
        LogicGenBaseVec subModuleOutputRepresent;
        LogicGenBaseVec subModuleInputRepresent;

        ///////////// recruit io of sub module/////////////////////////////////////
        for (ModuleGen* subMdGen: _subModulePool){
            /////////// output
            recruitLogicGenBase(subModuleOutputRepresent,
                                subMdGen->_genWires[WIRE_AUTO_GEN_OUTPUT]);
            for (LogicGenBase* mdOutput: subMdGen->_wirePoolWithOutputMarker){
                assert(mdOutput != nullptr);
                subModuleOutputRepresent.push_back(mdOutput);
            }
            ////////// input
            recruitLogicGenBase(subModuleInputRepresent,
                                subMdGen->_genWires[WIRE_AUTO_GEN_INPUT]);
            for (LogicGenBase* mdInput: subMdGen->_wirePoolWithInputMarker){
                assert(mdInput != nullptr);
                subModuleInputRepresent.push_back(mdInput);
            }
        }
        ///////////////////////////////////////////////////////////

        /**
         *
         *   module io declaration
         *
         */
        fileWriter->addData("//////////////////////////////////////////////////////////////////////////////\n"
                            "//////////////////// MODULE DEC ////////////////////////////////////////////////\n"
                            "////////////////////////////////////////////////////////////////////////////////\n"
                            );
        fileWriter->addData("module ");

        if (isExplicitMod){
            fileWriter->addData(explicitModName+"(\n");
        }else{
            fileWriter->addData(getOpr() + "(\n");
        }

        //////// declare input/output element
        std::vector<std::string> ioVec = getIoDec();
        ioVec.emplace_back("input wire clk");
        writeGenVec(ioVec, fileWriter, ",\n");
        fileWriter->addData("\n);\n");

        /***                               |   wire declaration   |   wire operation   |
         * -----------------------------------------------------------------------------
         * glob/auto/user input  wire ---->           no          |         no
         * glob/auto/user output wire ---->           no          |        yes
         * submodule input wire       ---->          yes          |        yes
         * submodule output wire      ---->          yes          |         no
         * -----------------------------------------------------------------------------
         * bridge wire                ---->          yes          |        yes
         * -----------------------------------------------------------------------------
         */

        /*
         * declare variable initiation
         *
         */
        fileWriter->addData("////regDecVar\n");
        writeGenVec(_regPool.getDecVars()                          , fileWriter, "\n"); fileWriter->addData("\n////wireDecVar\n");
        writeGenVec(_wirePool.getDecVars()                         , fileWriter, "\n"); fileWriter->addData("\n////_exprPool\n");
        writeGenVec(_exprPool.getDecVars()                         , fileWriter, "\n"); fileWriter->addData("\n////_nestPool\n");
        writeGenVec(_nestPool.getDecVars()                         , fileWriter, "\n"); fileWriter->addData("\n////_valPool\n");
        writeGenVec(_valPool.getDecVars()                          , fileWriter, "\n"); fileWriter->addData("\n////_memBlockPool\n");
        writeGenVec(_memBlockPool.getDecVars()                     , fileWriter, "\n"); fileWriter->addData("\n////_memBlockElePool\n");
        writeGenVec(_memBlockElePool.getDecVars()                  , fileWriter, "\n"); fileWriter->addData("\n////input of submodule\n");
        writeGenVec(subModuleInputRepresent.getDecVars()           , fileWriter, "\n"); fileWriter->addData("\n////output of submodule\n");
        writeGenVec(subModuleOutputRepresent.getDecVars()          , fileWriter, "\n"); fileWriter->addData("\n////bridgeVec\n");
        writeGenVec(_genWirePools[WIRE_AUTO_GEN_INTER].getDecVars(), fileWriter, "\n");fileWriter->addData("\n");

        fileWriter->addData("///////////////////////////////////////////////\n"
                            "//////////////////// operation///////////////////\n"
                            "/////////////////////////////////////////////////\n"
                            );



        fileWriter->addData("\n///regOp\n");
        writeGenVec(_regPool.getDecOps()                                , fileWriter, "\n"); fileWriter->addData("\n///_wirePoolOp\n");
        writeGenVec(_wirePool.getDecOps()                               , fileWriter, "\n"); fileWriter->addData("\n///_exprPoolOp\n");
        writeGenVec(_exprPool.getDecOps()                               , fileWriter, "\n"); fileWriter->addData("\n///_nestPoolOp\n");
        writeGenVec(_nestPool.getDecOps()                               , fileWriter, "\n"); fileWriter->addData("\n///_valPoolOp\n");
        writeGenVec(_valPool.getDecOps()                                , fileWriter, "\n"); fileWriter->addData("\n///_memBlockPoolOp\n");
        writeGenVec(_memBlockPool.getDecOps()                           , fileWriter, "\n"); fileWriter->addData("\n///_memBlockElePoolOp\n");
        writeGenVec(_memBlockElePool.getDecOps()                        , fileWriter, "\n"); fileWriter->addData("\n///output User VecOp\n");
        writeGenVec(_wirePoolWithOutputMarker.getDecOps()               , fileWriter, "\n"); fileWriter->addData("\n///outputVecOp\n");
        writeGenVec(_genWirePools[WIRE_AUTO_GEN_OUTPUT].getDecOps()     , fileWriter, "\n"); fileWriter->addData("\n///outputVecOp global\n");
        writeGenVec(_genWirePools[WIRE_AUTO_GEN_GLOB_OUTPUT].getDecOps(), fileWriter, "\n"); fileWriter->addData("\n////input of submodule\n");
        writeGenVec(subModuleInputRepresent.getDecOps()                 , fileWriter, "\n"); fileWriter->addData("\n///bridgeVecOp\n");
        writeGenVec(_genWirePools[WIRE_AUTO_GEN_INTER].getDecOps()      , fileWriter, "\n");
        ////////// declare submodule connectivity
        fileWriter->addData("/// sub module declaration\n");
        for (ModuleGen* subMdGen: _subModulePool){
            fileWriter->addData(subMdGen->getSubModuleDec(subMdGen, genStructure));
        }
        //////// end module
        fileWriter->addData("\nendmodule\n\n");

    }



    std::vector<std::string> ModuleGen::getIoDec(){

        std::vector<std::string> result;
        for (LogicGenBase* lgb: _wirePoolWithInputMarker                ){result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: _wirePoolWithOutputMarker               ){result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: _genWirePools[WIRE_AUTO_GEN_INPUT]      ){result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: _genWirePools[WIRE_AUTO_GEN_OUTPUT]     ){result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: _genWirePools[WIRE_AUTO_GEN_GLOB_INPUT] ){result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: _genWirePools[WIRE_AUTO_GEN_GLOB_OUTPUT]){result.push_back(lgb->decIo());}

        return result;
    }

    std::vector<std::string> ModuleGen::getParamDec(){

        std::vector<std::string> result;
        for (LogicGenBase* lgb: _pmValPool){result.push_back(lgb->decParamVal());}
        return result;

    }

    //////////////////////////// get module dec as sub
    ///
    std::string ModuleGen::getSubModuleDec(ModuleGen* subMdGen, GenStructure* genStructure){
        assert(genStructure != nullptr);
        assert(subMdGen != nullptr);

        std::vector<std::string> ioStrs;
        std::string result;

        /////// get the host module to generate
        ModuleGen* hostModuleGen = genStructure->getMasterModuleGen(subMdGen);

        ////// declare submodule
        result += hostModuleGen->getOpr() + "  ";
        result += subMdGen->getOpr() + " #( ";

        std::vector<std::string> ioParamVals;
        for (LogicGenBase* lgb: subMdGen->_pmValPool){
            ioParamVals.push_back(lgb->decOp());
        }
        result += joinStr(ioParamVals, ',');

        result += ") ";
        result += "(\n";

        ////////////////// declare input and output
        for (const std::string& inputStr : _wirePoolWithInputMarker.getOprs()){
            ioStrs.push_back(inputStr);
        }
        for (const std::string& outputStr: _wirePoolWithOutputMarker.getOprs()){
            ioStrs.push_back(outputStr);
        }
        for (const std::string& inputStr : _genWirePools[WIRE_AUTO_GEN_INPUT].getOprs()){
            ioStrs.push_back(inputStr);
        }
        for (const std::string& outputStr: _genWirePools[WIRE_AUTO_GEN_OUTPUT].getOprs()){
            ioStrs.push_back(outputStr);
        }
        ioStrs.push_back("clk");
        bool isFirst = true;
        for (const std::string& retStr: ioStrs){
            if (!isFirst){
                result += ",\n";
            }
            result += retStr;
            isFirst = false;
        }

        result += ");\n";
        return result;
    }

    std::string ModuleGen::getOpr(){
        return _master->getGlobalName() + "_" + _master->getVarName();
    }


}