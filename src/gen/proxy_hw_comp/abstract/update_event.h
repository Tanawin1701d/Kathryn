//
// Created by tanawin on 27/11/25.
//

#ifndef SRC_GEN_PROXYHWCOMP_ABSTRACT_UPDATEEVENT_H
#define SRC_GEN_PROXYHWCOMP_ABSTRACT_UPDATEEVENT_H

#include "cassert"
#include "vector"

#include "model/hw_component/abstract/operable.h"
#include "util/file_writer/code_writer/verilog_writer.h"


namespace kathryn{

    class ModuleGen;
    class AssignGenBase;
    struct UEBaseGenEngine{

        std::vector<UEBaseGenEngine*> sub_engine; /// it mainly used to delete

        virtual ~UEBaseGenEngine();

        ////// handle rerouting
        static  Operable* reroute_base(Operable* src_opr, ModuleGen* md_gen);
        static  void      reroute_and_replace(Operable*& src_opr, ModuleGen* md_gen);
        virtual void      reroute(ModuleGen* md_gen) = 0;
        virtual void      gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen) = 0;
        ///// gen_basic_connect use only simple connection
        virtual void      gen_basic_connect(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen) {assert(false);}


    };

    struct UpdateEventBasic;
    struct UEBasicGenEngine: UEBaseGenEngine{
        UpdateEventBasic* master;

        explicit UEBasicGenEngine(UpdateEventBasic* m): master(m){
            assert(m != nullptr);
        }

        bool validate_assign_sensivity() const;
        void reroute(ModuleGen* md_gen) override;
        void gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen) override;
        void gen_basic_connect(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen) override;

    };

    struct UpdateEventGrp;
    struct UEGrpGenEngine: UEBaseGenEngine{

        UpdateEventGrp* master = nullptr;

        explicit UEGrpGenEngine(UpdateEventGrp* m): master(m){
            assert(m != nullptr);
        }

        void reroute(ModuleGen* md_gen) override;
        void gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen) override;

    };

    struct UpdateEventCond;
    struct UECondGenEngine: UEBaseGenEngine{

        UpdateEventCond* master = nullptr;

        UECondGenEngine(UpdateEventCond* m): master(m){
            assert(m != nullptr);
        }

        void reroute(ModuleGen* md_gen) override;
        void gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen) override;
        void gen_basic_connect(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen) override;

    };

    struct UpdateEventSwitch;
    struct UESwitchGenEngine: UEBaseGenEngine{

        UpdateEventSwitch* master = nullptr;

        UESwitchGenEngine(UpdateEventSwitch* m): master(m){
            assert(m != nullptr);
        }


        void reroute(ModuleGen* md_gen) override;
        void gen_ass(CbBaseVerilog& cb_ver, AssignGenBase* assign_gen) override;

    };
}

#endif //SRC_GEN_PROXYHWCOMP_ABSTRACT_UPDATEEVENT_H