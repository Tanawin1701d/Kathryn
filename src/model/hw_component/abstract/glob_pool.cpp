//
// Created by tanawin on 26/6/2024.
//

#include "glob_pool.h"

#include "cassert"
#include "vector"
#include "set"

namespace kathryn{

    ////////// it is used to check how much wire is used
    std::set<std::string> glob_io_name;
    std::vector<WireMarker*>  glob_input_pool;
    std::vector<WireMarker*>  glob_output_pool;
    std::vector<WireMarker*>  md_input_pool;
    std::vector<WireMarker*>  md_output_pool;


    void add_to_glob_pool(WireMarker* src){
        assert(src != nullptr);
        check_is_there_io_name(src->get_glob_io_name());
        add_to_name_list(src->get_glob_io_name());
        if (src->get_marker() == WMT_GLOB_INPUT){
            glob_input_pool.push_back(src); return;
        }
        if (src->get_marker() == WMT_GLOB_OUTPUT){
            glob_output_pool.push_back(src); return;
        }
        if (src->get_marker() == WMT_INPUT_MD){
            md_input_pool.push_back(src); return;
        }
        if (src->get_marker() == WMT_OUTPUT_MD){
            md_output_pool.push_back(src); return;
        }
        assert(false);
    }

    std::vector<WireMarker*>& get_glob_pool(bool is_input){
        return is_input ? glob_input_pool: glob_output_pool;
    }

    std::vector<WireMarker*>& get_md_io_pool(bool is_input){
        return is_input ? md_input_pool: md_output_pool;
    }

    void clean_glob_pool(){
        glob_io_name    .clear();
        glob_input_pool .clear();
        glob_output_pool.clear();
        md_input_pool   .clear();
        md_output_pool  .clear();
    }

    bool check_is_there_io_name(const std::string& test){
        auto iter = glob_io_name.find(test);
        return iter != glob_io_name.end();
    }

    void add_to_name_list(const std::string& io_name){
        glob_io_name.insert(io_name);
    }


}
