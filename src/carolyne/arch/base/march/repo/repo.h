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

        class MarchRepo_Base{

            std::vector<FetchUTM_Base*>     _fetchTypes;
            std::vector<AllocUTM_Base*>     _allocTypes;
            std::vector<RsvUTM_Base*>       _rsvTypes  ;
            std::vector<ExecUnitTypeMeta*>  _execTypes ;
            std::vector<RobUTM_Base*>       _robTypes  ;
            PhysicalRegFileBase*            _phyFileBase = nullptr;

        public:

            [[nodiscard]] std::vector<FetchUTM_Base*>     getFetchTypes () const{return _fetchTypes;}
            [[nodiscard]]std::vector<AllocUTM_Base*>      getAllocTypes () const{return _allocTypes;}
            [[nodiscard]] std::vector<RsvUTM_Base*>       getRsvTypes   () const{return _rsvTypes;  }
            [[nodiscard]] std::vector<ExecUnitTypeMeta*>  getExecTypes  () const{return _execTypes; }
            [[nodiscard]] std::vector<RobUTM_Base*>       getRobTypes   () const{return _robTypes;  }
            [[nodiscard]] PhysicalRegFileBase*            getPhyFileBase() const{return _phyFileBase; }

            void addFetchTypes (FetchUTM_Base*       fetchType)  { _fetchTypes.push_back(fetchType);}
            void addAllocTypes (AllocUTM_Base*   allocType)  { _allocTypes.push_back(allocType);}
            void addRsvTypes   (RsvUTM_Base*     rsvType)    { _rsvTypes.push_back(rsvType);    }
            void addExecTypes  (ExecUnitTypeMeta*    execType)   { _execTypes.push_back(execType);  }
            void addRobTypes   (RobUTM_Base*     robType)    { _robTypes.push_back(robType);    }
            void setPhyFileBase(PhysicalRegFileBase* phyFileBase){ _phyFileBase = phyFileBase;      }

        };

    }


#endif //src_carolyne_arch_base_march_repo_REPO_H
