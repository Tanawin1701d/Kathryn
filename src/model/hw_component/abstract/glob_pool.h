//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBPOOL_H
#define GLOBPOOL_H

#include "wire_marker.h"
#include "operable.h"

namespace kathryn{


    void add_to_glob_pool(WireMarker* src);
    std::vector<WireMarker*>& get_glob_pool(bool is_input);
    std::vector<WireMarker*>& get_md_io_pool(bool is_input);

    void clean_glob_pool();

    bool check_is_there_io_name(const std::string& test);
    void add_to_name_list(const std::string& io_name);

}

#endif //GLOBPOOL_H
