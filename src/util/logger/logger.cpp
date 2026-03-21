//
// Created by tanawin on 12/12/2566.
//
#include "iostream"
#include "cassert"
#include "logger.h"
#include "model/debugger/model_debugger.h"


namespace kathryn{


    /****
     * LAGACY model logger is now abandoned
     * **/


    std::ofstream* out_file_mf = nullptr;
    std::ofstream* out_file_md = nullptr;

    std::vector<m_f_value> mf_storage;

    std::vector<std::string> md_storage_name;//// key
    std::vector<MdLogVal*> md_storage_vals;

    void init_md_debugger(){
        ///out_file_md = new std::ofstream("/media/tanawin/tanawin1701e/project2/Kathryn/KOut/MD.txt");
    }

    void init_mf_debugger(){

        ////out_file_mf = new std::ofstream("/media/tanawin/tanawin1701e/project2/Kathryn/KOut/MF.txt");

    }

    void print_md_log_val(std::ofstream* md_file, int ident, MdLogVal* md_log_val){

        assert(md_file != nullptr);
        assert(md_log_val != nullptr);

        for (auto &val: md_log_val->vals) {
            /** print ident*/
            for (int ident_cnt = 0; ident_cnt < ident; ident_cnt++) {
                *out_file_md << " ";
            }
            *out_file_md << val << "\n";
        }
        ident += 4;
        for (auto sub_md_log: md_log_val->sub_val)
            print_md_log_val(md_file, ident, sub_md_log);

    }

    void finalize_md_debugger(){

//        for (int i = 0; i < md_storage_name.size(); i++){
//            *out_file_md << "[ " << md_storage_vals[i] << " ]\n";
//            print_md_log_val(out_file_md, 0, md_storage_vals[i]);
//            *out_file_md << "------------------------------\n";
//        }
//
//        out_file_md->close();
//        delete out_file_md;
    }

    void flush_mf_debugger(){
        for (auto& mf_value : mf_storage){
            *out_file_mf << "[ " << mf_value.md_debug->get_md_ident_val()
                       << " ]    " << mf_value.debug_msg << "\n";
            *out_file_mf << "---------------------------\n";
        }
        mf_storage.clear();
        *out_file_mf << "---------------------------------------\n";
        *out_file_mf << "---------------------------------------\n";
        *out_file_mf << "----------------FLUSH------------------\n";
        *out_file_mf << "---------------------------------------\n";
        *out_file_mf << "---------------------------------------\n";
    }

    void finalize_mf_debugger(){

//        out_file_mf->close();
//        delete out_file_mf;
    }

    void log_mf(ModelDebuggable* md_debug,
               const std::string& debug_msg){
//        m_f_value x = { md_debug, debug_msg};
//        mf_storage.push_back(x);
    }

    void log_md(const std::string& md_name,
               MdLogVal* val){
//        assert(val != nullptr);
//        md_storage_name.push_back(md_name);
//        md_storage_vals.push_back(val);
    }


}