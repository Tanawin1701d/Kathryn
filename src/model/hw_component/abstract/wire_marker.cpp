//
// Created by tanawin on 26/6/2024.
//

#include "wire_marker.h"

#include "utility"
#include "glob_pool.h"

namespace kathryn{

void WireMarker::as_input_glob(std::string value){
    _marker = WMT_GLOB_INPUT;
    set_io_name(std::move(value));
    add_to_glob_pool(this);
}

void WireMarker::as_input(std::string value){
    _marker = WMT_INPUT_MD;
    set_io_name(std::move(value));
    add_to_glob_pool(this);
}

void WireMarker::as_output_glob(std::string value){
    _marker = WMT_GLOB_OUTPUT;
    set_io_name(std::move(value));
    add_to_glob_pool(this);
}

void WireMarker::as_output(std::string value){
    _marker = WMT_OUTPUT_MD;
    set_io_name(std::move(value));
    add_to_glob_pool(this);
}

void WireMarker::set_io_name(const std::string& value){
    if (value.empty()){
        Identifiable* origin = get_opr_from_glob_io_ptr()->cast_to_ident();
        assert(origin != nullptr);
        io_value = origin->get_global_name() + "_" + origin->get_var_meta().var_name;
    }else{
        io_value = value;
    }
}

std::string WireMarker::get_glob_io_name(){
    return io_value;
}



}