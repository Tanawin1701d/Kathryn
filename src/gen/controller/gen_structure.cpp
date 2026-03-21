//
// Created by tanawin on 3/7/2567.
//

#include "gen_structure.h"


namespace kathryn{

    //////////////////////////
    ///
    /// module checker
    ///
    //////////////////////////

    // bool ModuleChecker::is_same_module(ModuleGen* md_a, ModuleGen* md_b){
    //     assert(md_a != nullptr);
    //     assert(md_b != nullptr);
    //
    //     auto iter_a = _masterSrcModuleMap.find(md_a);
    //     auto iter_b = _masterSrcModuleMap.find(md_b);
    //
    //     assert(iter_a != _masterSrcModuleMap.end());
    //     assert(iter_b != _masterSrcModuleMap.end());
    //
    //     return iter_a->second == iter_b->second;
    // }
    //
    // void ModuleChecker::add_to_grp(ModuleGen* md_a){
    //     assert(_masterSrcModuleMap.find(md_a) == _masterSrcModuleMap.end());
    //     ///////// compare to all head
    //     int idx = 0;
    //     for (ModuleGen* md_gen: _masterSrcModule){
    //         bool result = md_gen->start_cmp_module(md_a, _master);
    //         if (result){break;}
    //         idx++;
    //     }
    //     if (idx >= _masterSrcModule.size()){
    //         _masterSrcModule.push_back(md_a);
    //     }
    //     _masterSrcModuleMap.insert({md_a, idx});
    // }
    //
    // ModuleGen* ModuleChecker::get_master_module_gen(ModuleGen* mdg){
    //     auto finder = _masterSrcModuleMap.find(mdg);
    //     assert(finder != _masterSrcModuleMap.end());
    //     int master_idx =  finder->second;
    //     assert(master_idx < _masterSrcModule.size());
    //     return _masterSrcModule[master_idx];
    // }
    //
    // //////////////////////////
    // ///
    // /// gen structure
    // ///
    // //////////////////////////
    //
    // bool GenStructure::is_the_same_module(ModuleGen* ma, ModuleGen* mb){
    //     assert(ma != nullptr);
    //     assert(mb != nullptr);
    //
    //     if (ma->get_glob_cerf() != mb->get_glob_cerf()){
    //         return false;
    //     }
    //
    //     module_global_cef md_glob_cerf = ma->get_glob_cerf();
    //     auto md_check_iter = _mdCheckStorage.find(md_glob_cerf);
    //     assert(md_check_iter != _mdCheckStorage.end());
    //
    //     bool eq_result = md_check_iter->second.is_same_module(ma, mb);
    //     return eq_result;
    // }
    //
    // void GenStructure::add_new_module(ModuleGen* new_md) {
    //     assert(new_md != nullptr);
    //
    //     module_global_cef md_glob_cerf = new_md->get_glob_cerf();
    //     auto md_check_iter = _mdCheckStorage.find(md_glob_cerf);
    //
    //     ////// create new one is there is no
    //     if (md_check_iter == _mdCheckStorage.end()){
    //         _mdCheckStorage.insert({md_glob_cerf, ModuleChecker(this)});
    //     }
    //
    //     ModuleChecker* md_checker = &_mdCheckStorage.find(md_glob_cerf)->second;
    //     md_checker->add_to_grp(new_md);
    //
    // }
    //
    // ModuleGen* GenStructure::get_master_module_gen(ModuleGen* mdg){
    //     assert(mdg != nullptr);
    //     module_global_cef glob_cerf = mdg->get_glob_cerf();
    //     auto iter = _mdCheckStorage.find(glob_cerf);
    //     /////// we must found master module
    //     assert(iter != _mdCheckStorage.end());
    //     ModuleChecker& module_checker = iter->second;
    //     ModuleGen* master_gen = module_checker.get_master_module_gen(mdg);
    //     assert(master_gen != nullptr);
    //     ///std::cout << "get gen " << mdg->get_cerf().var_meta.var_name << " return " << master_gen->get_cerf().var_meta.var_name << std::endl;
    //     return master_gen;
    // }
    //
    // std::vector<ModuleGen*> GenStructure::get_all_master_module_gen(){
    //     std::vector<ModuleGen*> result;
    //     for (auto iter: _mdCheckStorage){
    //         ModuleChecker& md_checker = iter.second;
    //         for (ModuleGen* master_module_gen : md_checker.get_uniq_md_gen()){
    //             assert(master_module_gen != nullptr);
    //             result.push_back(master_module_gen);
    //         }
    //     }
    //     return result;
    // }


}