//
// Created by tanawin on 9/1/2025.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_CARO_MARCH_REPO_CARO_REPO_H
#define KATHRYN_SRC_CAROLYNE_ARCH_CARO_MARCH_REPO_CARO_REPO_H

#include "carolyne/arch/base/util/regType.h"
#include "carolyne/arch/caro/march/pRegFile/caro_preg.h"
#include "carolyne/arch/base/march/repo/repo.h"
#include "carolyne/arch/caro/march/fetchUnit/caro_fetchMeta.h"
#include "carolyne/arch/caro/march/alloc/caro_allocMeta.h"
#include "carolyne/arch/caro/march/rsvUnit/caro_rsvMeta.h"
#include "carolyne/arch/caro/march/robUnit/caro_robMeta.h"

namespace kathryn::carolyne::caro{

    class MarchRepo_Caro: public MarchBaseRepo{


    public:

        explicit MarchRepo_Caro(){

            ///////// build physical register file
            auto* pregFile = new PRegFile();
            setPhyFileBase(pregFile);

            ////////// build fetch unit
            auto* fetchUnit = new FetchUTM();
            addFetchTypes(fetchUnit);

            ////////// alloc unit
            auto* allocUnit = new AllocUTM();
            addAllocTypes(allocUnit);

            ////////// reservation station
            auto* rsvUnit   = new RsvUTM();
            addRsvTypes(rsvUnit);

            ///////// reorder buffer
            auto* robUnit   = new RobUTM();
            addRobTypes(robUnit);

        }


    };

}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_CARO_MARCH_REPO_CARO_REPO_H
