//
// Created by tanawin on 22/11/2024.
//

#ifndef KATHRYN_SRC_MODEL_HWCOMPONENT_MODULE_MULTITHREADMODULE_MTAGENT_H
#define KATHRYN_SRC_MODEL_HWCOMPONENT_MODULE_MULTITHREADMODULE_MTAGENT_H

#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/wire/wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/flowBlock/loop/doWhileBase.h"
#include "model/flowBlock/loop/whileBase.h"
#include "model/flowBlock/cond/if.h"
#include "model/flowBlock/cond/elif.h"
#include "model/flowBlock/cond/zif.h"
#include "model/flowBlock/cond/zelif.h"
#include "model/flowBlock/seq/seq.h"
#include "model/flowBlock/par/par.h"
#include "model/flowBlock/time/wait.h"

namespace kathryn{

    typedef int mt_tid;

    enum mt_status{
        MTS_HALT,
        MTS_RUNNING
    };

    constexpr int MT_TID_SZ = 8;
    constexpr int DUMMY_TID = 0;

    struct MtCtrlAgent{

        mReg(curTid , MT_TID_SZ);
        mReg(nextTid, MT_TID_SZ);
        mReg(nextTidInScpReady, 1); ///// data is ready
        mReg(outScpHaveData, 1); //// space is ready to get in


        mReg(susReq  , 1);
        mReg(startReq, 1);

        /**mt ctrl is master of these thing*/
        mWire(reqMcToInScp , 1); ///// controller request Memory controller transfer to scratch pad
        mWire(finMcToInScp , 1);

        mWire(reqInScpToMd , 1); //// controller request scratch pad to model
        mWire(finInScpToMd , 1);

        mWire(reqMdToOutScp, 1); //// controlller request model transfer to out scratch pad
        mWire(finMdToOutScp, 1);

        mWire(reqOutScpToMc, 1);
        mWire(finOutScpToMc, 1);

        mWire(modelIsRunning, 1);
        /////////////////////////////////////////////




        void masterCtrlReqSuspend();
        void masterCtrlReqStart(Operable& tid); //// it will wait till model running
        void modelReqSuspend();

        Operable& getRunningStatus();

        //////// memory controller to In SCP
        Operable& isReqMcToInScp();
        void      ackMcToInScp();
        //////// In SCP to model
        Operable& isReqInScpToMd();
        void      ackInScpToMd();
        //////// model to Out SCP
        Operable& isReqMdToOutScp();
        void      ackMdToOutScp();
        //////// Out SCP to memory controller
        Operable& isReqOutScpToMc();
        void      ackOutScpToMc();

        void flow();

        void buildModelCtrl();
        void buildInDataCtrl();
        void buildOutDataCtrl();




    };


}

#endif //KATHRYN_SRC_MODEL_HWCOMPONENT_MODULE_MULTITHREADMODULE_MTAGENT_H
