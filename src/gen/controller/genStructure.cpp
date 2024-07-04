//
// Created by tanawin on 3/7/2567.
//

#include "genStructure.h"


namespace kathryn{

    //////////////////////////
    ///
    /// module checker
    ///
    //////////////////////////

    bool ModuleChecker::isSameModule(ModuleGen* mdA, ModuleGen* mdB){
        assert(mdA != nullptr);
        assert(mdB != nullptr);

        auto iterA = _masterSrcModuleMap.find(mdA);
        auto iterB = _masterSrcModuleMap.find(mdB);

        assert(iterA != _masterSrcModuleMap.end());
        assert(iterB != _masterSrcModuleMap.end());

        return iterA->second == iterB->second;
    }

    void ModuleChecker::addToGrp(ModuleGen* mdA){
        assert(_masterSrcModuleMap.find(mdA) == _masterSrcModuleMap.end());
        ///////// compare to all head
        int idx = 0;
        for (ModuleGen* mdGen: _masterSrcModule){
            bool result = mdGen->startCmpModule(mdA, _master);
            if (result){break;}
            idx++;
        }
        if (idx >= _masterSrcModule.size()){
            _masterSrcModule.push_back(mdA);
        }
        _masterSrcModuleMap.insert({mdA, idx});
    }

    ModuleGen* ModuleChecker::getMasterModuleGen(ModuleGen* mdg){
        auto finder = _masterSrcModuleMap.find(mdg);
        assert(finder != _masterSrcModuleMap.end());
        int masterIdx =  finder->second;
        assert(masterIdx < _masterSrcModule.size());
        return _masterSrcModule[masterIdx];
    }

    //////////////////////////
    ///
    /// gen structure
    ///
    //////////////////////////

    bool GenStructure::isTheSameModule(ModuleGen* ma, ModuleGen* mb){
        assert(ma != nullptr);
        assert(mb != nullptr);

        if (ma->getGlobCerf() != mb->getGlobCerf()){
            return false;
        }

        moduleGlobalCef mdGlobCerf = ma->getGlobCerf();
        auto mdCheckIter = _mdCheckStorage.find(mdGlobCerf);
        assert(mdCheckIter != _mdCheckStorage.end());

        bool eqResult = mdCheckIter->second.isSameModule(ma, mb);
        return eqResult;
    }

    void GenStructure::addNewModule(ModuleGen* newMd) {
        assert(newMd != nullptr);

        moduleGlobalCef mdGlobCerf = newMd->getGlobCerf();
        auto mdCheckIter = _mdCheckStorage.find(mdGlobCerf);

        ////// create new one is there is no
        if (mdCheckIter == _mdCheckStorage.end()){
            _mdCheckStorage.insert({mdGlobCerf, ModuleChecker(this)});
        }

        ModuleChecker* mdChecker = &_mdCheckStorage.find(mdGlobCerf)->second;
        mdChecker->addToGrp(newMd);

    }

    ModuleGen* GenStructure::getMasterModuleGen(ModuleGen* mdg){
        assert(mdg != nullptr);
        moduleGlobalCef globCerf = mdg->getGlobCerf();
        auto iter = _mdCheckStorage.find(globCerf);
        /////// we must found master module
        assert(iter != _mdCheckStorage.end());
        ModuleChecker& moduleChecker = iter->second;
        ModuleGen* masterGen = moduleChecker.getMasterModuleGen(mdg);
        assert(masterGen != nullptr);
        std::cout << "get gen " << mdg->getCerf().varMeta.varName << " return " << masterGen->getCerf().varMeta.varName << std::endl;
        return masterGen;
    }

    std::vector<ModuleGen*> GenStructure::getAllMasterModuleGen(){
        std::vector<ModuleGen*> result;
        for (auto iter: _mdCheckStorage){
            ModuleChecker& mdChecker = iter.second;
            for (ModuleGen* masterModuleGen : mdChecker.getUniqMdGen()){
                assert(masterModuleGen != nullptr);
                result.push_back(masterModuleGen);
            }
        }
        return result;
    }


}