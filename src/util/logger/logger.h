//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_LOGGER_H
#define KATHRYN_LOGGER_H

#include "iostream"
#include "fstream"
#include "string"
#include "vector"

namespace kathryn{

    /*** In the future we will build debug model more efficiently*/

    class ModelDebuggable;

    struct m_f_value{
        ModelDebuggable* md_debug;
        std::string      debug_msg;
    };

    struct MdLogVal{
        std::vector<std::string> vals;
        std::vector<MdLogVal*>   sub_val;

        void add_val(const std::string& val){ vals.push_back(val); }

        MdLogVal* make_new_sub_val(){
            auto sub = new MdLogVal();
            sub_val.push_back(sub);
            return sub;
        }


    };

    extern std::string out_path;
    extern std::vector<m_f_value> mf_storage;
    extern std::vector<MdLogVal*> md_storage_vals;
    extern std::vector<std::string> md_storage_name;//// key
    /////extern std::ofstream* out_file_mf;
    ////// MF model formation

    void init_md_debugger();
    void init_mf_debugger();

    void finalize_md_debugger();

    void flush_mf_debugger();
    void finalize_mf_debugger();


    void log_mf(ModelDebuggable* md_debug, const std::string& debug_msg);

    void log_md(const std::string& md_name, MdLogVal* val);


}

#endif //KATHRYN_LOGGER_H
