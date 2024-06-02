//
// Created by tanawin on 21/2/2567.
//

#ifndef KATHRYN_LOGICSIMENGINE_H
#define KATHRYN_LOGICSIMENGINE_H

#include "sim/simResWriter/simResWriter.h"
#include "model/simIntf/base/modelProxy.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/Slice.h"


namespace kathryn{

    class LogicSimEngine: public ModelProxyBuild, public ModelProxyRetrieve{

    protected:

        Assignable*  _asb        = nullptr;
        IdentBase*   _ident      = nullptr;
        VCD_SIG_TYPE _vcdSigType = VST_DUMMY;
        bool         _setToWrite = false;
        bool         _isTempReq  = false; ///// request temp request
        ull          _initVal    = 0;



    public:
        LogicSimEngine(Assignable* asb, IdentBase*   ident,
                       VCD_SIG_TYPE sigType, bool isTempReq,
                        ull initVal
                       );

        void proxyBuildInit() override;

        std::string getVarName() override{return _ident->getGlobalName();}

        ull         getVarId()   override{return _ident->getGlobalId();}

        void        setVCDWriteStatus(bool status){ _setToWrite = status;}

        std::string getVarNameFromOpr(Operable* opr);

        /*** c++ create section**/

        std::string createVariable()      override;

        std::string createOp()            override;

        std::string createMemorizeOp()    override;

        std::string registerToProxy()     override{return "";}

        std::string createMemBlkAssOp()   override{return "";}

        std::string collectData()         override{return "";}

        /////// TODO proxy
        ///
        /// TODO add vcd WRiter
        ///
        ///
        ValRep<MAX_VAL_REP_SIZE>* proxyRepA = nullptr;

        void proxyRetInit() override;
        virtual ValRep<MAX_VAL_REP_SIZE>* getProxyRep();
    };

    class LogicSimEngineInterface{
    public:
        virtual LogicSimEngine* getSimEngine() = 0;

        explicit operator ull(){
            ////// TODO
        }

        explicit operator ValRepBase(){
            ////// TODO
        }


    };

}

#endif //KATHRYN_LOGICSIMENGINE_H
