//
// Created by tanawin on 25/12/25.
//

#ifndef EXAMPLE_O3_SIMCOMPARE_SIMCTRLBASE_H
#define EXAMPLE_O3_SIMCOMPARE_SIMCTRLBASE_H

#include <fstream>
#include "kathryn.h"
#include "frontEnd/cmd/paramReader.h"
#include "simState.h"

namespace kathryn::o3{

    class O3SimCtrlBase{
    public:
        const int AMT_STAGE = 5;
        int _curTestCaseIdx = 0;
        std::string _prefixFolder;
        std::vector<std::string> _testTypes;
        uint32_t _imem      [IMEM_ROW]{};
        uint32_t _dmem      [DMEM_ROW]{};
        uint32_t _regTestVal[REG_NUM] {};

        bool     lastDmemEnable = false; //// enabler
        bool     lastDmemRead   = true;  //// used if enabler is true
        uint32_t lastDmemAddr   = 0;
        uint32_t lastDmemData   = 0;

        SlotWriterBase& _slotWriter;
        SimState& _state;

    public:
        explicit O3SimCtrlBase(CYCLE                    limitCycle,
                               const std::string&       prefix,
                               std::vector<std::string> testTypes,
                               SimProxyBuildMode        buildMode,
                               SlotWriterBase&          slotWriter,
                               SimState&                state);

        ////// memory management for each cycle

        void          resetDmem();


        ////// memory initialization
        virtual void  readAssembly (const std::string& filePath);
        virtual void  readAssertVal(const std::string& filePath);

        ///// on model action
        virtual void  readMem2Fetch         (){assert(false);} //// it has to place at the begin of cycle
        virtual void  readWriteDataMemGetCmd(){assert(false);} //// it has to place at the end of cycle
        virtual void  readWriteDataMemDoCmd (){assert(false);}  //// it has to place at the bigin of cycle
        virtual void  resetRegister         (){assert(false);}
        virtual void  testRegister          (){assert(false);}
        virtual void  postCycleAction       (){assert(false);}
    };

}

#endif //EXAMPLE_O3_SIMCOMPARE_SIMCTRLBASE_H