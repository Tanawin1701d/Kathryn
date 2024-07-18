//
// Created by tanawin on 21/2/2567.
//

#ifndef KATHRYN_LOGICSIMENGINE_H
#define KATHRYN_LOGICSIMENGINE_H

#include "sim/simResWriter/simResWriter.h"
#include "sim/modelSimEngine/base/modelProxy.h"
#include "sim/modelSimEngine/base/proxyEventBase.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/Slice.h"



namespace kathryn{

    class LogicSimEngine: public ModelProxyBuild, public ModelProxyRetrieve{

    protected:

        Assignable*   _asb        = nullptr;
        Identifiable* _ident      = nullptr;
        VCD_SIG_TYPE  _vcdSigType = VST_DUMMY;
        bool          _setToWrite = false;
        bool          _isTempReq  = false; ///// request temp request
        bool          _flowPerfBit= false; ////// it is used to set that this logic is used to be perf profiler
        ull           _initVal    = 0;

    public:
        ull createMask(Slice maskSlice);
        ///////////////////////// get opr value from opr
        std::string getSrcOprFromOpr(Operable* opr);
        std::string getSlicedSrcOprFromOpr(Operable* opr);
        std::string getSlicedAndShiftSrcOprFromOpr(Operable* opr, Slice desSlice);

        //////////////////////// gen main operation

        std::string genOpWithSoleCondition           (const std::string& auxAssStr = "");
        std::string genOpWithChainCondition          (const std::string& auxAssStr = "");
        std::string genAssignAEqB                    (Slice     desSlice, bool isDesTemp,
                                                      Operable* srcOpr);
        ///////////////////////// sliced opr
        virtual std::string genSrcOpr                ();
        virtual std::string genSlicedOprTo           (Slice srcSlice);
        virtual std::string genSlicedOprAndShift     (Slice desSlice, Slice srcSlice );


        LogicSimEngine(Assignable* asb, Identifiable*   ident,
                       VCD_SIG_TYPE sigType, bool isTempReq,
                        ull initVal
                       );

        void proxyBuildInit() override;

        std::string              getVarName()      override;
        std::vector<std::string> getRegisVarName() override;
        std::string              getTempVarName();
        ull                      getVarId()        override{return _ident->getGlobalId();}
        void                     setVCDWriteStatus(bool status){ _setToWrite = status;}

        VCD_SIG_TYPE             getSigType() const {return _vcdSigType;}
        Slice                    getSize()    const {return _asb->getAssignSlice();}


        /*** c++ create section**/

        std::string createGlobalVariable() override;
        std::string createLocalVariable()  override{return "";}
        std::string createOp()             override;
        std::string createOpEndCycle()     override{return "";}
        std::string createOpEndCycle2()    override;


        bool        isUserDeclare()       override{return _ident->isUserVar();}

        void        setFlowBlockIden(bool flowIden){_flowPerfBit = flowIden;}

        /////// proxy
        ///

        void proxyRetInit(ProxySimEventBase* modelSimEvent)       override;
        ValRepBase* getProxyRep() override;
    };

    class LogicSimEngineInterface{
    public:
        virtual LogicSimEngine* getSimEngine() = 0;
    };

}

#endif //KATHRYN_LOGICSIMENGINE_H
