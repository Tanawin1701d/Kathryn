//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_repo_REPO_H
#define src_carolyne_arch_base_march_repo_REPO_H
#include "carolyne/arch/base/march/execUnit/execMetaBase.h"
#include "carolyne/arch/base/march/fetchUnit/fetchMetaBase.h"
#include "carolyne/arch/base/march/pRegFile/physicalRegFile_base.h"
#include "carolyne/arch/base/march/robUnit/robMetaBase.h"


namespace kathryn::carolyne{

        /***
         * This class is the main storage for micro-architecture meta-data that designer specify
         */

        class MarchRepo{

            FetchMeta*                  _fetchMeta     = nullptr;
            RobMeta*                    _robMeta       = nullptr;
            PhysicalRegFileBase*        _phyRegFiles   = nullptr;
            std::vector<ExecUnitMeta*>  _execUnitMetas;

        public:

            /**setter*/
            void setFetchMeta    (FetchMeta*           fetchMeta   ) { _fetchMeta = fetchMeta; }
            void setRobMeta      (RobMeta*             robMeta     ) { _robMeta = robMeta; }
            void setPhyRegFiles  (PhysicalRegFileBase* phyRegFiles ) { _phyRegFiles = phyRegFiles; }
            void addExecUnitMeta (ExecUnitMeta*        execUnit    ) { _execUnitMetas.push_back(execUnit); }

            /** getter function*/
            FetchMeta*                 getFetchMeta    () {return _fetchMeta;}
            RobMeta*                   getRobMeta      () {return _robMeta;}
            PhysicalRegFileBase*       getPhyRegFiles  () {return _phyRegFiles;}
            std::vector<ExecUnitMeta*> getExecUnitMetas() {return _execUnitMetas;}

        };

    }


#endif //src_carolyne_arch_base_march_repo_REPO_H
