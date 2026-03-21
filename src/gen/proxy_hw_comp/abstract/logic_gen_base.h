//
// Created by tanawin on 20/6/2024.
//


#ifndef KATHRYN_GEN_PROXYHWCOMP_ABSTRCT
#define KATHRYN_GEN_PROXYHWCOMP_ABSTRCT

#include "utility"
#include "util/file_writer/file_writer_base.h"
#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/abstract/wire_marker.h"

namespace kathryn{

    enum OUT_SEARCH_POL{
        SUBMOD,
        MASTERMOD
    };

    /** compare Stategy
     * CP_LC_ONLY
     * CP_
     */
    class ModuleGen;
    class LogicGenBase{
    protected:
        ModuleGen*     _mdGenMaster = nullptr;
        Assignable*    _asb = nullptr;
        Identifiable*  _ident = nullptr;

    public:
        explicit LogicGenBase(ModuleGen*    md_gen_master,
                              Assignable*   asb,
                              Identifiable* ident
        );
        virtual ~LogicGenBase() = default;
        std::string get_opr_str_from_opr(Operable* opr1);
        std::string get_opr_str_from_opr_and_shink_msb(Operable* opr1, int target_size);
        ///////// routing zone
        virtual void route_dep() {assert(false);} ///// do routing
        ///////// generate cerificate
        //virtual void gen_cerf(MODULE_GEN_GRP mgg, int grp_idx, int idx);
        ///////// start compare
        //virtual bool compare(LogicGenBase* lgb) = 0;
        ///////// get zone
        virtual std::string get_opr();
        virtual std::string get_opr(Slice sl);
        //////// gen zone
        virtual std::string  dec_glob_io()   {assert(false);}
        virtual std::string  dec_glob_io_asm(){assert(false);}
        virtual std::string  dec_param_val() {assert(false);}
        virtual std::string  dec_io()       {assert(false);}
        virtual std::string  dec_variable() {assert(false);}
        virtual std::string  dec_op()       {assert(false);}
        /////// glob io check
        virtual WIRE_MARKER_TYPE  get_glob_io_status(){return WIRE_MARKER_TYPE::WMT_NONE;}

        ///////// getter
        [[nodiscard]] ModuleGen*get_module_gen_ptr() const{
            assert(_mdGenMaster != nullptr);
            return _mdGenMaster;
        }
        [[nodiscard]] Identifiable*get_ident_ptr()    const {return _ident;}

        virtual void add_direct_update_event(UpdateEventBase* ueb){assert(false);}

    };


    class LogicGenBaseVec: public std::vector<LogicGenBase*>{
    public:
        void route_dep_all(){
            for (auto& x : *this){
                x->route_dep();
            }
        }

        std::vector<std::string> get_oprs(){
            std::vector<std::string> result;
            for (auto& x : *this){
                result.push_back(x->get_opr());
            }
            return result;
        }

        std::vector<std::string> get_dec_ios(){
            std::vector<std::string> result;
            for (auto& x : *this){
                result.push_back(x->dec_io());
            }
            return result;
        }

        std::vector<std::string> get_dec_vars(){
            std::vector<std::string> result;
            for (auto& x : *this){
                result.push_back(x->dec_variable());
            }
            return result;
        }

        std::vector<std::string> get_dec_ops(){
            std::vector<std::string> result;
            for (auto& x : *this){
                result.push_back(x->dec_op());
            }
            return result;
        }
    };

    inline void write_gen_vec(const std::vector<std::string>& src,
                     FileWriterBase* fw,
                     const std::string& sep){
        assert(fw != nullptr);
        bool is_first = true;
        for (const std::string& x: src){
            if (x.empty()){
                continue;
            }
            if(!is_first){
                fw->add_data(sep);
            }
            fw->add_data(x);
            is_first = false;
        }
    }

    class LogicGenInterface{
    public:
        virtual               ~LogicGenInterface() = default;
        virtual void          create_logic_gen()     = 0;
        virtual LogicGenBase* get_logic_gen_ptr()        = 0;
    };

}


#endif
