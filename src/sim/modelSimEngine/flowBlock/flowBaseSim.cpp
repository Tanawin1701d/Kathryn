//
// Created by tanawin on 17/4/2567.
//

#include "flowBaseSim.h"

#include "sim/controller/simController.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
#include "sim/modelSimEngine/base/proxyEventBase.h"
#include "util/str/strUtil.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"


namespace kathryn{
    FlowBaseSimEngine::FlowBaseSimEngine(FlowBlockBase* flowBlockBase):
        _flowBlockBase(flowBlockBase){
        assert(_flowBlockBase != nullptr);
    }

    ValR FlowBaseSimEngine::getValRep(){
        return {SIM_VALREP_TYPE_ALL(bitSizeOfUll),
            bitSizeOfUll,
            "PERF_" + _flowBlockBase->get_global_name()};
    }

    ValR FlowBaseSimEngine::getVarNameCurStatus(){
        ValR base = getValRep();
        return {SIM_VALREP_TYPE_ALL(1), 1, base.getData()+"_CURBIT"};
    }

    std::vector<std::string> FlowBaseSimEngine::getRegisVarName(){
        return {getValRep().getData(), getVarNameCurStatus().getData()};
    }

    ull FlowBaseSimEngine::getVarId(){
        return _flowBlockBase->get_global_id();
    }

    SIM_VALREP_TYPE_ALL FlowBaseSimEngine::getValR_Type(){
        return SIM_VALREP_TYPE_ALL(bitSizeOfUll);
    }

    CLOCK_MODE FlowBaseSimEngine::getClockMode(){return CM_CLK_UNUSED;}


    void FlowBaseSimEngine::getRecurVarName(std::vector<std::string>& result){
        result.push_back(getValRep().getData());
        for (FlowBlockBase* fb : _flowBlockBase->get_sub_blocks_ref()){
            FlowBaseSimEngine* subBlockSimEngine = fb->get_sim_engine();
            subBlockSimEngine->getRecurVarName(result);
        }
        for (FlowBlockBase* fb : _flowBlockBase->get_con_blocks_ref()){
            FlowBaseSimEngine* conBlockSimEngine = fb->get_sim_engine();
            conBlockSimEngine->getRecurVarName(result);
        }
    }

    void FlowBaseSimEngine::getRecurVarNameCurStsatus(std::vector<std::string>& result){
        result.push_back(getVarNameCurStatus().getData());
        for (FlowBlockBase* fb : _flowBlockBase->get_sub_blocks_ref()){
            FlowBaseSimEngine* subBlockSimEngine = fb->get_sim_engine();
            subBlockSimEngine->getRecurVarNameCurStsatus(result);
        }
        for (FlowBlockBase* fb : _flowBlockBase->get_con_blocks_ref()){
            FlowBaseSimEngine* conBlockSimEngine = fb->get_sim_engine();
            conBlockSimEngine->getRecurVarNameCurStsatus(result);
        }
    }


    void FlowBaseSimEngine::createGlobalVariable(CbBaseCxx& cb){

        cb.addSt(getValRep().buildVar(0));
        cb.addSt(getVarNameCurStatus().buildVar(0));


        for (FlowBlockBase* fb : _flowBlockBase->get_sub_blocks_ref()){
            FlowBaseSimEngine* subBlockSimEngine = fb->get_sim_engine();
            subBlockSimEngine->createGlobalVariable(cb);
        }
        for (FlowBlockBase* fb : _flowBlockBase->get_con_blocks_ref()){
            FlowBaseSimEngine* conBlockSimEngine = fb->get_sim_engine();
            conBlockSimEngine->createGlobalVariable(cb);
        }
    }

    void FlowBaseSimEngine::createOp(CbBaseCxx& cb){

        cb.addCm("////////////////////////////////////");
        cb.addCm(_flowBlockBase->get_global_name());
        ////////////////////////////////////////////////////////////////////////////
        //////////// subBlock build
        ////////////////////////////////////////////////////////////////////////////
        for (FlowBlockBase* fb : _flowBlockBase->get_sub_blocks_ref()){
            FlowBaseSimEngine* subBlockSimEngine = fb->get_sim_engine();
            subBlockSimEngine->createOp(cb.addSubBlock());
        }

        /////////////////////////////////////////////////////////////////////////////
        ///////////// this block purpose
        /////////////////////////////////////////////////////////////////////////////
        cb.addSt(getVarNameCurStatus().eq(ValR(SIM_VALREP_TYPE_ALL(1), 1, "0")).toString());

        //////////// basic node recruitment
        cb.addCm("basic node rc");
        for (Node* sysNode : _flowBlockBase->get_sys_nodes_ref()){
            assert(sysNode != nullptr);
            for (CtrlFlowRegBase* stateReg : sysNode->get_cycle_related_reg()){
                if (stateReg != nullptr){
                    ValR stateRegRep = stateReg->getSimEngine()->getValRep();
                    ValR checkRegRep =
                        ValR(SIM_VALREP_TYPE_ALL(stateRegRep.getSize()), stateRegRep.getSize(), "0");
                    cb.addSt( getVarNameCurStatus()
                        .eq(getVarNameCurStatus() | (stateRegRep != checkRegRep))
                        .toString());
                }
            }
        }

        ///////////// sub block recruitment
        cb.addCm("sub block recruitment");
        for (FlowBlockBase* subFb : _flowBlockBase->get_sub_blocks_ref()){
            FlowBaseSimEngine* subBlockSimEngine = subFb->get_sim_engine();
            cb.addSt( getVarNameCurStatus()
                .eq(getVarNameCurStatus() | subBlockSimEngine->getVarNameCurStatus())
                .toString()
            );
            for(FlowBlockBase* conOfSubFb: subFb->get_con_blocks_ref()){
                FlowBaseSimEngine* conOfSubFbSimEngine = conOfSubFb->get_sim_engine();
                cb.addSt( getVarNameCurStatus()
                  .eq(getVarNameCurStatus() | conOfSubFbSimEngine->getVarNameCurStatus())
                  .toString()
              );
            }

        }
        cb.addSt(
                getValRep().eq(
                        (getValRep()+
                        getVarNameCurStatus().cast(SIM_VALREP_TYPE_ALL(bitSizeOfUll), bitSizeOfUll)
                        )
                 ).toString());


        ////////////////////////////////////////////////////////////////////////////
        /////////////// conblock block purpose
        ////////////////////////////////////////////////////////////////////////////

        //////////// do for con block

        for (FlowBlockBase* fb : _flowBlockBase->get_con_blocks_ref()){
            FlowBaseSimEngine* conBlockSimEngine = fb->get_sim_engine();
            conBlockSimEngine->createOp(cb);
        }

        cb.addCm("////////////////////////////////////");
    }

    //////////////////// return initiate
    ///
    void FlowBaseSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getValPerf(getValRep().getData());
        proxyRep.setSize(bitSizeOfUll);

        _proxyRepCurBit = modelSimEvent->getValPerf(getVarNameCurStatus().getData());
        _proxyRepCurBit.setSize(1);
        ///////// subblock init
        for (FlowBlockBase* subBlock : _flowBlockBase->get_sub_blocks_ref()){
            subBlock->get_sim_engine()->proxyRetInit(modelSimEvent);
        }
        ///////// conblock init
        for (FlowBlockBase* conBlock : _flowBlockBase->get_con_blocks_ref()){
            conBlock->get_sim_engine()->proxyRetInit(modelSimEvent);
        }
    }

    ValRepBase& FlowBaseSimEngine::getProxyRep(){
        return proxyRep;
    }

    bool FlowBaseSimEngine::isBlockRunning(){
        assert(_proxyRepCurBit.isInUsed());
        return _proxyRepCurBit.getVal();
    }
}
