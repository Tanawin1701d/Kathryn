//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_KATHRYN_H
#define KATHRYN_KATHRYN_H


#include "model/controller/controller.h"
#include "gen/controller/genController.h"
#include "frontEnd/cmd/paramReader.h"
#include "sim/interface/simInterface.h"

#include "util/fileWriter/slotWriter/slotWriter.h"
#include "model/hwComponent/abstract/globPool.h"
#include "util/logger/logger.h"

namespace kathryn{

    void startModelKathryn();
    void resetKathryn();
    PARAM readParamKathryn(std::string filePath);

}

#endif //KATHRYN_KATHRYN_H
