//
// Created by tanawin on 20/6/2024.
//

#ifndef MODULEGEN_H
#define MODULEGEN_H

#include "unordered_map"
#include "gen/proxy_hw_comp/expression/expr_gen.h"
#include "gen/proxy_hw_comp/expression/nest_gen.h"
#include "gen/proxy_hw_comp/register/reg_gen.h"
#include "gen/proxy_hw_comp/value/value_gen.h"
#include "gen/proxy_hw_comp/wire/wire_gen.h"
#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"


#include "model/hw_component/mem_block/mem_block.h"
#include "model/hw_component/register/register.h"
#include "model/hw_component/wire/wire_auto.h"
#include "model/hw_component/wire/wire_sub_type.h"
#include "model/hw_component/expression/expression.h"
#include "model/hw_component/value/value.h"
#include "model/hw_component/expression/nest.h"
#include "model/flow_block/abstract/sp_reg/wait_reg.h"
#include "util/file_writer/file_writer_group.h"


namespace kathryn{


    enum MODULE_GEN_PROGRESS{
        MGP_UNINIT,
        MGP_INITED_ELE,
        MGP_ROUTED,
        MGP_MARK_PRE_WRITE,
        MGP_MARK_PRE_REDUNDANT
    };


    class Module;
    class GenStructure;
    class ModuleGen{
    protected:
        Module*        _master           = nullptr;
        int            depth_from_global_module = 0;
        MODULE_GEN_PROGRESS _mgp = MGP_UNINIT;

    public:
        LogicGenBaseVec   _regPool;
        LogicGenBaseVec   _wirePool;
        LogicGenBaseVec   _wirePoolWithInputMarker; //// with marker that mark as input of the module (only module)
        LogicGenBaseVec   _wirePoolWithOutputMarker; //// with marker that mark as output of the module (only module)
        LogicGenBaseVec   _exprPool;
        LogicGenBaseVec   _nestPool;
        LogicGenBaseVec   _valPool;
        LogicGenBaseVec   _pmValPool;
        LogicGenBaseVec   _memBlockPool;
        LogicGenBaseVec   _memBlockElePool;
        ///// gen_wire_pool it can be use after finalize Route Ele is used
        LogicGenBaseVec   _genWirePools[WIRE_AUTO_GEN_CNT];

        ////// gen_wire_map gen engine must create when wire is build and
        /// add to the structure
        std::unordered_map<Operable*, int> _genWireMaps[WIRE_AUTO_GEN_CNT];
        std::vector<WireAuto*>             _genWires   [WIRE_AUTO_GEN_CNT];
        std::vector<ModuleGen*>            _subModulePool;

        explicit ModuleGen(Module* master);

        MODULE_GEN_PROGRESS get_gen_progress()const {return _mgp;}
        Module*get_master_module_ptr() const {return _master;}

        /*
         * main progress
         */

        void start_init_ele    ();
        void start_route_ele   ();
        void finalize_route_ele();

        void start_write_file_master(bool               require_new_file,
                                  FileWriterBase*    upper_file_writer,
                                  FileWriterGroup*   writer_group,
                                  bool               is_explicit_mod,
                                  const std::string& explicit_mod_name
                                  );

        void start_write_file(FileWriterBase* file_writer,
                            const std::string& explicit_mod_name);

        /*
         * routing operation
         */
        WireAuto* gen_auto_wire_base(Operable* opr1, Operable* real_src,
                                     const std::string& wire_name,
                                     WIRE_AUTO_GEN_TYPE wire_gen_type,
                                     bool               connect_the_wire = true);
        bool         is_there_auto_gen_wire(Operable* real_src, WIRE_AUTO_GEN_TYPE wire_gen_type);
        WireAuto* get_auto_gen_wire    (Operable* real_src, WIRE_AUTO_GEN_TYPE wire_gen_type);
        //// route opr1
        Operable* route_src_opr_to_this_module (Operable* exact_real_src);
        int       get_dept() const{return depth_from_global_module;}
        //// global io
        std::vector<WireAuto*>& get_auto_gen_wire_refs (WIRE_AUTO_GEN_TYPE gen_wire_type){
            return _genWires[gen_wire_type];
        }

        /**
         * file generator
         */
        std::vector<std::string> get_io_dec();
        std::vector<std::string> get_param_dec();
        std::string get_sub_module_dec(ModuleGen* md_gen);
        std::string get_opr();

        ////// ^------ it should be called from get_sub_module_dec
        /**
         *  cmp function
         */

        /**
         * recruit function
         */

        template<typename T>
        void create_logic_gen_base(std::vector<T*>& srcs){
            for(T* src: srcs){
                src->create_logic_gen();
            }
        }

        template<typename T>
        void recruit_logic_gen_base(LogicGenBaseVec& des,
                                 std::vector<T*>& srcs){
            for(T* src: srcs){
                LogicGenBase* logic_gen_base = src->get_logic_gen_ptr();
                assert(logic_gen_base != nullptr);
                des.push_back(logic_gen_base);
            }
        }

        template<typename T>
        LogicGenBaseVec recruit_logic_gen_base(std::vector<T*>& srcs){
            LogicGenBaseVec result;
            recruit_logic_gen_base(result, srcs);
            return result;
        }

        template<typename T>
        void create_and_recruit_logic_gen_base(LogicGenBaseVec& des,
        std::vector<T*>& srcs){
            create_logic_gen_base(srcs);
            recruit_logic_gen_base(des, srcs);
        }
    };

    class ModuleGenInterface{
    public:
        virtual ~ModuleGenInterface() = default;
        virtual void       create_module_gen() = 0;
        virtual ModuleGen*get_module_gen_ptr() = 0;

    };

}

#endif //MODULEGEN_H