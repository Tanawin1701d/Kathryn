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
#include "util/fileWriter/codeWriter/cppWriter.h"



namespace kathryn{

    class LogicSimEngine: public ModelProxyBuild, public ModelProxyRetrieve{

    protected:

        Assignable*   _asb                = nullptr;
        Identifiable* _ident              = nullptr;
        VCD_SIG_TYPE  _vcdSigType         = VST_DUMMY;
        bool          _setToWrite         = false;
        bool          _isTempReq          = false; ///// request temp request
        bool          _flowPerfBit        = false; ////// it is used to set that this logic is used to be perf profiler
        ull           _initVal            = 0;
        bool          _reqGlobDec         = false;

    public:
        //////////////////////// gen main operation

        void createOpWithSoleCondition               (CbBaseCxx& cb, const std::string& auxAssStr = "");
        void genOpWithChainCondition                 (CbBaseCxx& cb, const std::string& auxAssStr = "");
        std::string genAssignAEqB                    (Slice     desSlice, bool isDesTemp,
                                                      Operable* srcOpr);
        ///////////////////////// sliced opr
        virtual ValR genSrcOpr                ();
        virtual ValR genSlicedOprTo           (Slice srcSlice, SIM_VALREP_TYPE_ALL desField);
        virtual ValR genSlicedOprAndShift     (Slice desSlice, Slice srcSlice, SIM_VALREP_TYPE_ALL desField);


        LogicSimEngine(Assignable* asb, Identifiable*   ident,
                       VCD_SIG_TYPE sigType, bool isTempReq,
                        ull initVal
                       );

        void proxyBuildInit() override;

        //std::string              getVarName()      override;
        ValR                     getValRep() override;
        std::vector<std::string> getRegisVarName() override;
        ValR                     getTempValRep();
        ull                      getVarId()        override{return _ident->getGlobalId();}
        SIM_VALREP_TYPE_ALL      getValR_Type()    override;
        void                     setVCDWriteStatus(bool status){ _setToWrite = status;}

        [[nodiscard]]
        VCD_SIG_TYPE             getSigType() const {return _vcdSigType;}
        [[nodiscard]]
        Slice                    getSize()    const {return _asb->getAssignSlice();}


        /*** c++ create section**/

        void createGlobalVariable(CbBaseCxx& cb) override;
        void createLocalVariable (CbBaseCxx& cb) override{}
        void createOp            (CbBaseCxx& cb) override;
        void createOpEndCycle    (CbBaseCxx& cb) override{}
        void createOpEndCycle2   (CbBaseCxx& cb) override;


        bool        isUserDeclare()       override{return _ident->isUserVar();}

        void        setFlowBlockIden(bool flowIden){_flowPerfBit = flowIden;}
        void        reqGlobDec(){ _reqGlobDec = true;}

        /////// proxy
        ///

        void proxyRetInit(ProxySimEventBase* modelSimEvent)       override;
        ValRepBase& getProxyRep() override;
    };

    class LogicSimEngineInterface{
    public:
        virtual ~LogicSimEngineInterface() = default;
        virtual LogicSimEngine* getSimEngine() = 0;
    };

}

#endif //KATHRYN_LOGICSIMENGINE_H
