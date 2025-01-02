//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_repo_REPO_H
#define src_carolyne_arch_base_march_repo_REPO_H
#include "carolyne/arch/base/march/fetchUnit/fetchMetaBase.h"
#include "carolyne/arch/base/march/alloc/allocMetaBase.h"
#include "carolyne/arch/base/march/execUnit/execMetaBase.h"
#include "carolyne/arch/base/march/pRegFile/physicalRegFile_base.h"
#include "carolyne/arch/base/march/robUnit/robMetaBase.h"

#include<vector>

namespace kathryn::carolyne{



        /***
         * This class is the main storage for micro-architecture meta-data TYPE specification
         * that designer specify
         */

        class MarchRepo{

            std::vector<FetchTypeMeta*>     _fetchTypes;
            std::vector<AllocUnitTypeMeta*> _allocTypes;
            std::vector<RsvUnitTypeMeta*>   _rsvTypes  ;
            std::vector<ExecUnitTypeMeta*>  _execTypes ;
            std::vector<RobUnitTypeMeta*>   _robTypes  ;
            PhysicalRegFileBase*            _phyFileBase = nullptr;

        public:

            [[nodiscard]] std::vector<FetchTypeMeta*>     getFetchTypes () const{return _fetchTypes;}
            [[nodiscard]]std::vector<AllocUnitTypeMeta*>  getAllocTypes () const{return _allocTypes;}
            [[nodiscard]] std::vector<RsvUnitTypeMeta*>   getRsvTypes   () const{return _rsvTypes;  }
            [[nodiscard]] std::vector<ExecUnitTypeMeta*>  getExecTypes  () const{return _execTypes; }
            [[nodiscard]] std::vector<RobUnitTypeMeta*>   getRobTypes   () const{return _robTypes;  }
            [[nodiscard]] PhysicalRegFileBase*            getPhyFileBase() const{return _phyFileBase; }

            void addFetchTypes (FetchTypeMeta*       fetchType)  { _fetchTypes.push_back(fetchType);}
            void addAllocTypes (AllocUnitTypeMeta*   allocType)  { _allocTypes.push_back(allocType);}
            void addRsvTypes   (RsvUnitTypeMeta*     rsvType)    { _rsvTypes.push_back(rsvType);    }
            void addExecTypes  (ExecUnitTypeMeta*    execType)   { _execTypes.push_back(execType);  }
            void addRobTypes   (RobUnitTypeMeta*     robType)    { _robTypes.push_back(robType);    }
            void setPhyFileBase(PhysicalRegFileBase* phyFileBase){ _phyFileBase = phyFileBase;      }

        };

    }


#endif //src_carolyne_arch_base_march_repo_REPO_H
