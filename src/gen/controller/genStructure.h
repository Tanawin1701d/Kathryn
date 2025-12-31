//
// Created by tanawin on 3/7/2567.
//

#ifndef KATHRYN_GENSTRUCTURE_H
#define KATHRYN_GENSTRUCTURE_H

#include "gen/proxyHwComp/module/moduleGen.h"

namespace kathryn{

    /**
     * TO DO compare system is ABANDON
     *
     ***/


    // struct GenStructure;
    // struct ModuleChecker{
    //     //////// it collect distict only if there new module
    //     //////// that have the same the addToGrp should not be added
    //     //////// to vector
    //     GenStructure* _master = nullptr;
    //     std::vector<ModuleGen*>   _masterSrcModule;
    //     std::map<ModuleGen*, int> _masterSrcModuleMap;
    //
    //     explicit ModuleChecker(GenStructure* master): _master(master){
    //         assert(master != nullptr);
    //     }
    //
    //     bool isSameModule(ModuleGen* mdA, ModuleGen* mdB); ////// is two module is same
    //     void addToGrp(ModuleGen* mdA); ////// the added mdgen is not allowed
    //     std::vector<ModuleGen*>&
    //     getUniqMdGen(){return _masterSrcModule;}
    //     ModuleGen* getMasterModuleGen(ModuleGen* mdg);
    //
    // };
    //
    // struct GenStructure{
    //     std::map<moduleGlobalCef, ModuleChecker> _mdCheckStorage;
    //     ////// check inside system do not do any compare
    //     bool isTheSameModule(ModuleGen* ma, ModuleGen* mb);
    //     void addNewModule(ModuleGen* newMd);
    //     ModuleGen* getMasterModuleGen(ModuleGen* mdg);
    //     std::vector<ModuleGen*> getAllMasterModuleGen();
    //     void reset() {_mdCheckStorage.clear();}
    //
    // };
}

#endif //KATHRYN_GENSTRUCTURE_H
