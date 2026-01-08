//
// Created by tanawin on 25/12/25.
//

#include "simCtrlBase.h"


namespace kathryn::o3{

    O3SimCtrlBase::O3SimCtrlBase(CYCLE                    limitCycle,
                                 const std::string&       prefix,
                                 std::vector<std::string> testTypes,
                                 SimProxyBuildMode        buildMode,
                                 SlotWriterBase&          slotWriter,
                                 SimState&                state):


    _prefixFolder(prefix),
    _testTypes   (testTypes),
    _slotWriter  (slotWriter),
    _state       (state){}


    bool O3SimCtrlBase::isExecFin(){

        if ((!lastDmemRead) && (lastDmemAddr == 8) && (lastDmemWData == 1)){
            return true;
        }
        return false;

    }



    void O3SimCtrlBase::resetDmem(){
        std::memset(_dmem, 0, sizeof(_dmem));
        lastDmemEnable  = false;
        lastDmemRead    = true;
        lastDmemAddr    = 0;
        lastDmemWData    = 0;
    }

    bool O3SimCtrlBase::compareMemOp(O3SimCtrlBase& rhs){

        if (!lastDmemEnable){return true;}
        bool result = true;

        if (lastDmemRead != rhs.lastDmemRead){
            std::cout << TC_RED
                      << "Dmem read misMatch Kride got: "
                      << lastDmemRead
                      << "    Ride got: "
                      << rhs.lastDmemRead
                      << std::endl;
            result = false;
        }
        if (lastDmemAddr != rhs.lastDmemAddr){
            std::cout << TC_RED
                      << "Dmem addr misMatch Kride got: "
                      << cvtNum2HexStr(lastDmemAddr)
                      << "    Ride got: "
                      << cvtNum2HexStr(rhs.lastDmemAddr)
                      << std::endl;
            result = false;
        }

        if (!lastDmemRead){ ///// it is write
            if (lastDmemWData != rhs.lastDmemWData){
                std::cout << TC_RED
                          << "Dmem write Data misMatch Kride got: "
                          << cvtNum2HexStr(lastDmemWData)
                          << "    Ride got: "
                          << cvtNum2HexStr(rhs.lastDmemWData)
                          << std::endl;
            }
        }
        return result;

    }

    uint32_t O3SimCtrlBase::readAssemblyBase(const std::string& filePath,
                                             uint32_t* memPtr,
                                             uint32_t numRow){

        ////////// initialize file
        std::ifstream asmFile(filePath, std::ios::binary);
        if (!asmFile.is_open()){assert(false);}
        asmFile.seekg(0, std::ios::end);
        std::streampos fileSize = asmFile.tellg();
        assert((fileSize % 4) == 0);
        assert(fileSize <= (numRow << 2));
        asmFile.seekg(0, std::ios::beg);

        /** read instruction from file and write it to memory block*/
        uint32_t writeAddr = 0;
        uint32_t instr;
        while(asmFile.read(reinterpret_cast<char*>(&instr), sizeof instr)){
            memPtr[writeAddr] = instr;
            writeAddr++;
        }
        asmFile.close();

        ///// fill all with zero
        std::fill(memPtr + writeAddr, memPtr + numRow, 0);

        return numRow - writeAddr; //// the result is in byte * 4
    }



    void O3SimCtrlBase::readAssembly(const std::string& filePath){
        std::cout << TC_BLUE << "initialize IMEM" << TC_DEF << std::endl;
        uint32_t iremainRow = readAssemblyBase(filePath, _imem, IMEM_ROW);
        std::cout << TC_BLUE << "remainRow: " << iremainRow << TC_DEF << std::endl;
        std::cout << TC_GREEN << "initialize IMEM finish" << TC_DEF << std::endl;

        std::cout << TC_BLUE << "initialize DMEM" << TC_DEF << std::endl;
        uint32_t dremainRow = readAssemblyBase(filePath, _dmem, DMEM_ROW);
        std::cout << TC_BLUE << "remainRow: " << dremainRow << TC_DEF << std::endl;
        std::cout << TC_GREEN << "initialize IMEM finish" << TC_DEF << std::endl;
    }

    void O3SimCtrlBase::readAssertVal(const std::string& filePath){

        std::vector<std::string> rawVals;
        FileReaderBase reader(filePath);
        rawVals = reader.readLines();

        assert(rawVals.size() == REG_NUM);

        for (int regIdx = 0; regIdx < REG_NUM; regIdx++){
            _regTestVal[regIdx] = stoul(rawVals[regIdx]);
        }

    }

}