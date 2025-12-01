//
// Created by tanawin on 27/11/25.
//

#ifndef SRC_GEN_PROXYHWCOMP_ABSTRACT_UPDATEEVENT_H
#define SRC_GEN_PROXYHWCOMP_ABSTRACT_UPDATEEVENT_H

#include <cassert>
#include <vector>

#include "model/hwComponent/abstract/operable.h"
#include "util/fileWriter/codeWriter/verilogWriter.h"


namespace kathryn{

    class ModuleGen;
    class AssignGenBase;
    struct UEBaseGenEngine{

        std::vector<UEBaseGenEngine*> subEngine; /// it mainly used to delete

        virtual ~UEBaseGenEngine();

        ////// handle rerouting
        static  Operable* rerouteBase(Operable* srcOpr, ModuleGen* mdGen);
        static  void      rerouteAndReplace(Operable*& srcOpr, ModuleGen* mdGen);
        virtual void      reroute(ModuleGen* mdGen) = 0;
        virtual void      genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen) = 0;
        ///// genBasicConnect use only simple connection
        virtual void      genBasicConnect(CbBaseVerilog& cbVer, AssignGenBase* assignGen) {assert(false);}


    };

    struct UpdateEventBasic;
    struct UEBasicGenEngine: UEBaseGenEngine{
        UpdateEventBasic* master;

        explicit UEBasicGenEngine(UpdateEventBasic* m): master(m){
            assert(m != nullptr);
        }

        bool validateAssignSensivity() const;
        void reroute(ModuleGen* mdGen) override;
        void genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen) override;
        void genBasicConnect(CbBaseVerilog& cbVer, AssignGenBase* assignGen) override;

    };

    struct UpdateEventGrp;
    struct UEGrpGenEngine: UEBaseGenEngine{

        UpdateEventGrp* master = nullptr;

        explicit UEGrpGenEngine(UpdateEventGrp* m): master(m){
            assert(m != nullptr);
        }

        void reroute(ModuleGen* mdGen) override;
        void genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen) override;

    };

    struct UpdateEventCond;
    struct UECondGenEngine: UEBaseGenEngine{

        UpdateEventCond* master = nullptr;

        UECondGenEngine(UpdateEventCond* m): master(m){
            assert(m != nullptr);
        }

        void reroute(ModuleGen* mdGen) override;
        void genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen) override;

    };

    struct UpdateEventSwitch;
    struct UESwitchGenEngine: UEBaseGenEngine{

        UpdateEventSwitch* master = nullptr;

        UESwitchGenEngine(UpdateEventSwitch* m): master(m){
            assert(m != nullptr);
        }

        void reroute(ModuleGen* mdGen) override;
        void genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen) override;

    };
}

#endif //SRC_GEN_PROXYHWCOMP_ABSTRACT_UPDATEEVENT_H