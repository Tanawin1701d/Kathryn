//
// Created by tanawin on 1/7/2024.
//
#include "model/hwComponent/module/module.h"

namespace kathryn{


    void ModuleGen::startWriteFile(FileWriterBase* fileWriter){
        LogicGenBaseVec globInputVec;
        LogicGenBaseVec globOutputVec;
        LogicGenBaseVec inputVec;
        LogicGenBaseVec outputVec;
        LogicGenBaseVec bridgeVec;
        LogicGenBaseVec subModuleOutputRepresent;
        LogicGenBaseVec subModuleInputRepresent;

        ///////////// recruit io /////////////////////////////////////
        recruitLogicGenBase(globInputVec , _globalInputs);
        recruitLogicGenBase(globOutputVec, _globalOutputs);
        recruitLogicGenBase(inputVec     , _autoInputWires);
        recruitLogicGenBase(outputVec    , _autoOutputWires);
        recruitLogicGenBase(bridgeVec    , _interWires);
        for (ModuleGen* subMdGen: _subModulePool){
            recruitLogicGenBase(subModuleOutputRepresent,
                                subMdGen->_autoOutputWires);
            recruitLogicGenBase(subModuleInputRepresent,
                                subMdGen->_autoInputWires);
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
        fileWriter->addData(_master->getGlobalName() + "(\n");

        //////// declare input/output element
        std::vector<std::string> ioVec = getIoDec(inputVec,outputVec,globInputVec,globOutputVec);
        ioVec.emplace_back("input wire clk");
        writeGenVec(ioVec, fileWriter, ",\n");
        fileWriter->addData("\n);\n");

        /***                          |   wire declaration   |   wire operation   |
         * ------------------------------------------------------------------------
         * glob/auto input  wire ---->           no          |         no
         * glob/auto output wire ---->           no          |        yes
         * submodule input wire  ---->          yes          |        yes
         * submodule output wire ---->          yes          |         no
         * ------------------------------------------------------------------------
         * bridge wire           ---->          yes          |        yes
         * ------------------------------------------------------------------------
         */

        /*
         * declare variable initiation
         *
         */
        fileWriter->addData("////regDecVar\n");
        writeGenVec(_regPool.getDecVars()                , fileWriter, "\n"); fileWriter->addData("\n////wireDecVar\n");
        writeGenVec(_wirePool.getDecVars()               , fileWriter, "\n"); fileWriter->addData("\n////_exprPool\n");
        writeGenVec(_exprPool.getDecVars()               , fileWriter, "\n"); fileWriter->addData("\n////_nestPool\n");
        writeGenVec(_nestPool.getDecVars()               , fileWriter, "\n"); fileWriter->addData("\n////_valPool\n");
        writeGenVec(_valPool.getDecVars()                , fileWriter, "\n"); fileWriter->addData("\n////_memBlockPool\n");
        writeGenVec(_memBlockPool.getDecVars()           , fileWriter, "\n"); fileWriter->addData("\n////_memBlockElePool\n");
        writeGenVec(_memBlockElePool.getDecVars()        , fileWriter, "\n"); fileWriter->addData("\n////input of submodule\n");
        writeGenVec(subModuleInputRepresent.getDecVars() , fileWriter, "\n"); fileWriter->addData("\n////output of submodule\n");
        writeGenVec(subModuleOutputRepresent.getDecVars(), fileWriter, "\n"); fileWriter->addData("\n////bridgeVec\n");
        writeGenVec(bridgeVec.getDecVars()               , fileWriter, "\n");

        fileWriter->addData("///////////////////////////////////////////////\n"
                            "//////////////////// operation///////////////////\n"
                            "/////////////////////////////////////////////////\n"
                            );



        fileWriter->addData("\n///regOp\n");
        writeGenVec(_regPool.getDecOps()                 , fileWriter, "\n"); fileWriter->addData("\n///_wirePoolOp\n");
        writeGenVec(_wirePool.getDecOps()                , fileWriter, "\n"); fileWriter->addData("\n///_exprPoolOp\n");
        writeGenVec(_exprPool.getDecOps()                , fileWriter, "\n"); fileWriter->addData("\n///_nestPoolOp\n");
        writeGenVec(_nestPool.getDecOps()                , fileWriter, "\n"); fileWriter->addData("\n///_valPoolOp\n");
        writeGenVec(_valPool.getDecOps()                 , fileWriter, "\n"); fileWriter->addData("\n///_memBlockPoolOp\n");
        writeGenVec(_memBlockPool.getDecOps()            , fileWriter, "\n"); fileWriter->addData("\n///_memBlockElePoolOp\n");
        writeGenVec(_memBlockElePool.getDecOps()         , fileWriter, "\n"); fileWriter->addData("\n///outputVecOp\n");
        writeGenVec(outputVec.getDecOps()                , fileWriter, "\n"); fileWriter->addData("\n///outputVecOp global\n");
        writeGenVec(globOutputVec.getDecOps()            , fileWriter, "\n"); fileWriter->addData("\n////input of submodule\n");
        writeGenVec(subModuleInputRepresent.getDecOps()  , fileWriter, "\n"); fileWriter->addData("\n///bridgeVecOp\n");
        writeGenVec(bridgeVec.getDecOps()                , fileWriter, "\n");
        ////////// declare submodule connectivity
        fileWriter->addData("/// sub module declaration\n");
        for (ModuleGen* subMdGen: _subModulePool){
            fileWriter->addData(subMdGen->getSubModuleDec(subMdGen));
        }
        //////// end module
        fileWriter->addData("\nendmodule\n\n");

        for (ModuleGen* subMdGen: _subModulePool){
            subMdGen->startWriteFile(fileWriter);
        }
    }



    std::vector<std::string> ModuleGen::getIoDec(
        const LogicGenBaseVec& inputVec,
        const LogicGenBaseVec& outputVec,
        const LogicGenBaseVec& globInputVec,
        const LogicGenBaseVec& globOutputVec){

        std::vector<std::string> result;

        for (LogicGenBase* lgb: inputVec)     {result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: outputVec)    {result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: globInputVec) {result.push_back(lgb->decIo());}
        for (LogicGenBase* lgb: globOutputVec){result.push_back(lgb->decIo());}

        return result;
    }

    //////////////////////////// get module dec as sub
    ///
    std::string ModuleGen::getSubModuleDec(ModuleGen* subMdGen){
        assert(subMdGen != nullptr);

        std::vector<std::string> retStrs;
        std::string result;

        result += subMdGen->getOpr() + "  ";
        result += subMdGen->getOpr();
        result += "(\n";

        ////////////////// declare input and output
        LogicGenBaseVec inputGenEle;
        LogicGenBaseVec outputGenEle;
        recruitLogicGenBase(inputGenEle, _autoInputWires);
        recruitLogicGenBase(outputGenEle, _autoOutputWires);

        for (const std::string& inputStr : inputGenEle.getOprs()){
            retStrs.push_back(inputStr);
        }
        for (const std::string& outputStr: outputGenEle.getOprs()){
            retStrs.push_back(outputStr);
        }
        retStrs.push_back("clk");

        bool isFirst = true;

        for (const std::string& retStr: retStrs){
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
        return _master->getGlobalName();
    }


}