//
// Created by tanawin on 3/7/2567.
//

#ifndef KATHRYN_GENSTRUCTURE_H
#define KATHRYN_GENSTRUCTURE_H

#include "gen/proxy_hw_comp/module/module_gen.h"

namespace kathryn{

    /**
     * TO DO compare system is ABANDON
     *
     ***/


    // struct GenStructure;
    // struct ModuleChecker{
    //     //////// it collect distict only if there new module
    //     //////// that have the same the add_to_grp should not be added
    //     //////// to vector
    //     GenStructure* _master = nullptr;
    //     std::vector<ModuleGen*>   _masterSrcModule;
    //     std::map<ModuleGen*, int> _masterSrcModuleMap;
    //
    //     explicit ModuleChecker(GenStructure* master): _master(master){
    //         assert(master != nullptr);
    //     }
    //
    //     bool is_same_module(ModuleGen* md_a, ModuleGen* md_b); ////// is two module is same
    //     void add_to_grp(ModuleGen* md_a); ////// the added mdgen is not allowed
    //     std::vector<ModuleGen*>&
    //     get_uniq_md_gen(){return _masterSrcModule;}
    //     ModuleGen*get_master_module_gen_ptr(ModuleGen* mdg);
    //
    // };
    //
    // struct GenStructure{
    //     std::map<module_global_cef, ModuleChecker> _mdCheckStorage;
    //     ////// check inside system do not do any compare
    //     bool is_the_same_module(ModuleGen* ma, ModuleGen* mb);
    //     void add_new_module(ModuleGen* new_md);
    //     ModuleGen*get_master_module_gen_ptr(ModuleGen* mdg);
    //     std::vector<ModuleGen*> get_all_master_module_gen();
    //     void reset() {_mdCheckStorage.clear();}
    //
    // };
}

#endif //KATHRYN_GENSTRUCTURE_H
