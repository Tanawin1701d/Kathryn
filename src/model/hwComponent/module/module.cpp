//
// Created by tanawin on 30/11/2566.
//

#include "module.h"

#include "model/controller/controller.h"


namespace kathryn{


    Module::Module(bool initComp):
            Identifiable(TYPE_MODULE),
            HwCompControllerItf(),
            ModuleSimInterface()
    {
        if (initComp)
            com_init();
    }

    Module::~Module() {
        ///deleteSubElement(_stateRegs);
        for(auto& _spReg: _spRegs){
            deleteSubElement(_spReg);
        }
        deleteSubElement(_userRegs);
        deleteSubElement(_userWires);
        deleteSubElement(_userExpressions);
        deleteSubElement(_userVals);
        deleteSubElement(_userSubModule);
        deleteSubElement(_flowBlockBases);
    }

    void Module::com_init() {
        ctrl->on_module_init_components(this);
        /**post finalize component must be handle when object is buit finish*/
    }

    void Module::com_final() {
        /** invoke controller for design flow acknowledgement*/
        ctrl->on_module_init_designFlow(this);
        /** fix slave elements to belong to this module*/
        ctrl->on_module_final(this);
    }

    template<typename T>
    void Module::localizeSlaveVector(std::vector<T> &_vec) {
        for (size_t i = 0; i < _vec.size(); i++){
            _vec[i]->setLocalId((ull)i);
            _vec[i]->setParent(this);
        }
    }

    void Module::localizeSlaveElements() {
        /** state reg and user reg used same local ID sequence*/
        for (auto & _spReg : _spRegs)
            localizeSlaveVector(_spReg);
        localizeSlaveVector(_userRegs);
        localizeSlaveVector(_userWires);
        localizeSlaveVector(_userExpressions);
        localizeSlaveVector(_userVals);
        localizeSlaveVector(_userSubModule);
    }

    /**
     *
     * metas data pusher
     *
     * */
    /** todo may be check their are reg in the system */
    void Module::addSpReg(Reg* reg, SP_REG_TYPE spRegType){
        assert(reg != nullptr); /// can't be nullptr
        assert(spRegType < SP_CNT_REG);
        _spRegs[spRegType].push_back(reg);
    }

    void Module::addFlowBlock(FlowBlockBase* fb) {
        assert(fb != nullptr);
        _flowBlockBases.push_back(fb);
    }

    void Module::addUserReg(Reg* reg) {
        assert(reg != nullptr);
        _userRegs.push_back(reg);
    }

    void Module::addUserWires(Wire* wire) {
        assert(wire != nullptr);
        _userWires.push_back(wire);
    }

    void Module::addUserExpression(expression* expr) {
        assert(expr != nullptr);
        _userExpressions.push_back(expr);
    }

    void Module::addUserVal(Val* val) {
        assert(val != nullptr);
        _userVals.push_back(val);
    }

    void Module::addUserMemBlk(MemBlock* memBlock) {
        assert(memBlock != nullptr);
        _userMemBlks.push_back(memBlock);
    }

    void Module::addUserSubModule(Module* smd) {
        assert(smd != nullptr);
        _userSubModule.push_back(smd);
    }

    void Module::buildFlow() {
        /** build all hardware to flowBlock*/
        /** every flowblock will auto build when block is detach*/
        /** create nodewrap of all flowblock*/
        /** may be if block is top flow we must clear the stack*/
        ctrl->tryPurifyFlowStack();
        assert(ctrl->isAllFlowStackEmpty());
        std::vector<NodeWrap*> frontNodeWrap;

        for (auto fb: _flowBlockBases){
            frontNodeWrap.push_back(fb->sumarizeBlock());
        }
        for (auto nw: frontNodeWrap){
            /** we will have start wire node to start node*/
            nw->addDependNodeToAllNode(startNode);
            nw->assignAllNode();
            /** assume that node wrap that appear to module is not used anymore. */
        }

        /**check short circuit*/
        for(auto expr: _userExpressions){
            expr->startCheckShortCircuit();
        }
        for(auto wire: _userWires){
            wire->startCheckShortCircuit();
        }

    }


    std::string
    Module::getMdDescribe(){

        for (auto fb: _flowBlockBases){
            //logMD(fb->getMdIdentVal(), fb->getMdDescribe());
        }

        return "";

    }

    void Module::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + getMdIdentVal() + " ]");
        for (auto sb : _flowBlockBases){
            auto subLog = mdLogVal->makeNewSubVal();
            sb->addMdLog(subLog);
        }

    }

    /** override simulation*/
    void Module::beforePrepareSim(VcdWriter* vcdWriter, FlowColEle* flowColEle){

        assert(vcdWriter);
        /**RTL BEFORE PREPARE SIM(SP)*/
        for (int i = 0; i < SP_CNT_REG; i++){
            beforePrepareSimSubElement_RTL_only(_spRegs[i], vcdWriter);
        }
        /**RTL BEFORE PREPARE SIM(USER)*/
        beforePrepareSimSubElement_RTL_only(_userRegs, vcdWriter);
        beforePrepareSimSubElement_RTL_only(_userWires, vcdWriter);
        beforePrepareSimSubElement_RTL_only(_userExpressions, vcdWriter);
        beforePrepareSimSubElement_RTL_only(_userVals, vcdWriter);
        /**flow block prepare sim*/
        beforePrepareSimSubElement_FB_only(_flowBlockBases, flowColEle->populateSubEle());
        /**COMPLEX BEFORE PREPARE SUB SIM*/
        for(auto modulePtr : _userSubModule){
            assert(modulePtr != nullptr);
            modulePtr->beforePrepareSim(vcdWriter, flowColEle->populateSubEle());
        }



    }

    void Module::prepareSim(){

        /**RTL PREPARE SIM(SP)*/
        for (int i = 0; i < SP_CNT_REG; i++){
            prepareSimSubElement(_spRegs[i]);
        }
        /**RTL PREPARE SIM(USER)*/
        prepareSimSubElement(_userRegs);
        prepareSimSubElement(_userWires);
        prepareSimSubElement(_userExpressions);
        prepareSimSubElement(_userVals);
        prepareSimSubElement(_userMemBlks);
        /**COMPLEX PREPARE SUB SIM*/
        prepareSimSubElement(_userSubModule);
        ////// flow block not need prepare sim
    }


    /***/


    void Module::simStartCurCycle() {
        /**simulate rtl first*/

        /**RTL SIM(SP)*/
        for (int i = 0; i < SP_CNT_REG; i++){
            simStartCurSubElement(_spRegs[i]);
        }
        /**RTL SIM(USER)*/
        simStartCurSubElement(_userRegs);
        simStartCurSubElement(_userWires);
        simStartCurSubElement(_userExpressions);
        simStartCurSubElement(_userVals);
        simStartCurSubElement(_userMemBlks);
        /**COMPLEX SUB SIM*/
        simStartCurSubElement(_userSubModule);
        /**for now we are sure that the other is simulated*/
        simStartCurSubElement(_flowBlockBases);

        /**simulate flow block in which node is implicitly invoked*/
    }

    void Module::simStartNextCycle() {
        /**RTL SIM(SP)*/
        for (int i = 0; i < SP_CNT_REG; i++){
            simStartNextSubElement(_spRegs[i]);
        }
        /**RTL SIM(USER)*/
        simStartNextSubElement(_userRegs);
        simStartNextSubElement(_userMemBlks);
        /**COMPLEX SUB SIM*/
        simStartNextSubElement(_userSubModule);
    }

    void Module::curCycleCollectData() {
        /**RTL SIM(SP)*/
        for (int i = 0; i < SP_CNT_REG; i++){
            curCollectData(_spRegs[i]);
        }
        /**RTL SIM(USER)*/
        curCollectData(_userRegs);
        curCollectData(_userWires);
        curCollectData(_userExpressions);
        curCollectData(_userVals);
        /**COMPLEX SUB SIM*/
        curCollectData(_userSubModule);
        curCollectData(_flowBlockBases);
    }



    void Module::simExitCurCycle() {

        /**RTL SIM(SP)*/
        /** exit rtl first*/
        for (int i = 0; i < SP_CNT_REG; i++){
            simExitSubElement(_spRegs[i]);
        }
        /**RTL SIM(USER)*/
        simExitSubElement(_userRegs);
        simExitSubElement(_userWires);
        simExitSubElement(_userExpressions);
        simExitSubElement(_userVals);
        simExitSubElement(_userMemBlks);
        /**COMPLEX SUB SIM*/
        simExitSubElement(_userSubModule);
        simExitSubElement(_flowBlockBases);

        /** exit flowblock first*/
    }





    /** sub element interface*/

    template<typename T>
    void Module::beforePrepareSimSubElement_RTL_only(std::vector<T*>& subEleVec, VcdWriter* writer){
        for (auto ele: subEleVec){
            assert(ele != nullptr);
            ele->beforePrepareSim(
                    {true, /// for now
                     ele->concat_inheritName()+ "_" + ele->getVarName(),
                     writer
                    });
            ele->sortUpEventByPriority();
        }
    }

    template<typename T>
    void Module::prepareSimSubElement(std::vector<T*>& subEleVec){
        for (auto ele: subEleVec){
            assert(ele != nullptr);
            ele->prepareSim();
        }
    }

    template<typename T>
    void Module::simStartCurSubElement(std::vector<T*>& subEleVec){
        for (auto ele: subEleVec){
            assert(ele != nullptr);
            ele->simStartCurCycle();
        }
    }

    template<typename T>
    void Module::simStartNextSubElement(std::vector<T*>& subEleVec){
        for(auto ele: subEleVec){
            assert(ele != nullptr);
            ele->simStartNextCycle();
        }
    }

    template<typename T>
    void Module::curCollectData(std::vector<T*>& subEleVec){
        for (auto ele: subEleVec){
            ele->curCycleCollectData();
        }
    }

    template<typename T>
    void Module::simExitSubElement(std::vector<T*>& subEleVec){
        for (auto ele: subEleVec){
            ele->simExitCurCycle();
        }
    }



//    void Module::log(){
//        /***dfs in all flowBLock and sub FLow block*/
//        struct DFS_STATUS{
//            FlowBlockBase* fbb = nullptr;
//            int nextId = 0;
//        };
//
//        std::stack<DFS_STATUS> dfsSt;
//        ///std::string ret = "[ MODULE " + getIdentDebugValue() + " ]\n";
//        std::string ret;
//        for (auto fbbIter = _flowBlockBases.rbegin();
//            fbbIter != _flowBlockBases.rend();
//            fbbIter++
//        ){
//            dfsSt.push({*fbbIter, -1});
//        }
//
//        while (!dfsSt.empty()){
//            auto& top = dfsSt.top();
//
//            if (top.nextId == -1){
//                ret += top.fbb->getDescribe();
//                ret += "\n";
//            }
//
//            top.nextId++;
//
//            if (top.nextId == top.fbb->getSubBlocks().size()){
//                dfsSt.pop();
//            }else{
//                dfsSt.push({top.fbb->getSubBlocks()[top.nextId],
//                            -1});
//            }
//
//        }
//
//        std::string ident = "MODULE " + getIdentDebugValue();
//
//        logMD(ident,ret);
//
//    }


}