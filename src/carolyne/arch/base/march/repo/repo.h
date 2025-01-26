//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_repo_REPO_H
#define src_carolyne_arch_base_march_repo_REPO_H
#include "carolyne/arch/base/march/fetchUnit/fetchMetaBase.h"
#include "carolyne/arch/base/march/alloc/allocMetaBase.h"
#include "carolyne/arch/base/march/execUnit/execMetaBase.h"
#include "carolyne/arch/base/march/prfUnit/prfMetaBase.h"
#include "carolyne/arch/base/march/robUnit/robMetaBase.h"

#include<vector>

namespace kathryn::carolyne{



        /***
         * This class is the main storage for micro-architecture meta-data TYPE specification
         * that designer specify
         */

        class MarchBaseRepo{

        protected:

            std::vector<FetchUTM_Base*>     _fetchTypes;
            std::vector<AllocUTM_Base*>     _allocTypes;
            std::vector<RsvUTM_Base*>       _rsvTypes  ;
            std::vector<ExecUTM*>           _execTypes ;
            std::vector<RobUTM_Base*>       _robTypes  ;
            std::vector<PhyRegFileUTM_Base*>     _phyFileBases;

        public:

            [[nodiscard]] std::vector<FetchUTM_Base*>     getFetchTypes () const{return _fetchTypes;  }
            [[nodiscard]]std::vector<AllocUTM_Base*>      getAllocTypes () const{return _allocTypes;  }
            [[nodiscard]] std::vector<RsvUTM_Base*>       getRsvTypes   () const{return _rsvTypes;    }
            [[nodiscard]] std::vector<ExecUTM*>           getExecTypes  () const{return _execTypes;   }
            [[nodiscard]] std::vector<RobUTM_Base*>       getRobTypes   () const{return _robTypes;    }
            [[nodiscard]] std::vector<PhyRegFileUTM_Base*>     getPhyFileBase() const{return _phyFileBases;}

            void addFetchType  (FetchUTM_Base* fetchType  ) { _fetchTypes  .push_back(fetchType);  }
            void addAllocType  (AllocUTM_Base* allocType  ) { _allocTypes  .push_back(allocType);  }
            void addRsvType    (RsvUTM_Base*   rsvType    ) { _rsvTypes    .push_back(rsvType);    }
            void addExecType   (ExecUTM*       execType   ) { _execTypes   .push_back(execType);   }
            void addRobType    (RobUTM_Base*   robType    ) { _robTypes    .push_back(robType);    }
            void addPhyFileBase(PhyRegFileUTM_Base* phyFileBase) { _phyFileBases.push_back(phyFileBase);}
            void addExecTypes  (const std::vector<ExecUTM*>& execTypes){
                for (ExecUTM* execUtm: execTypes){addExecType(execUtm);}
            }

        };

    }


#endif //src_carolyne_arch_base_march_repo_REPO_H
