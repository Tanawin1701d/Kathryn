//
// Created by tanawin on 30/11/2566.
//

#include "module.h"

#include "model/controller/controller.h"


namespace kathryn{

    Wire* rstWire = &_make<Wire>("rstWire", 1);
    StartNode* startNode = new StartNode(rstWire);

    Module::Module(bool initComp):
            Identifiable(TYPE_MODULE),
            HwCompControllerItf(),
            FlowSimulatable(new FlowSimEngine())
    {
        if (initComp)
            com_init();
    }

    Module::~Module() {
        ///deleteSubElement(_stateRegs);
        for(auto& _spReg: _spRegs){
            deleteSubElement(_spReg);
        }
        deleteSubElement(_flowBlockBases);
        deleteSubElement(_userRegs);
        deleteSubElement(_userWires);
        deleteSubElement(_userExpressions);
        deleteSubElement(_userVals);
        deleteSubElement(_userSubModule);
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

    void Module::addUserSubModule(Module* smd) {
        assert(smd != nullptr);
        _userSubModule.push_back(smd);
    }

    void Module::buildFlow() {
        /** build all hardware to flowBlock*/
        /** every flowblock will auto build when block is detach*/
        /** create nodewrap of all flowblock*/
        /** may be if block is top flow we must clear the stack*/
        ctrl->purifyFlowStack();
        std::vector<NodeWrap*> frontNodeWrap;

        for (auto fb: _flowBlockBases){
            frontNodeWrap.push_back(fb->sumarizeBlock());
        }
        for (auto nw: frontNodeWrap){
            /** we will have start wire node to start node*/
            nw->addDependNodeToAllNode(startNode);
            nw->setAllDependNodeCond(BITWISE_AND);
            nw->assignAllNode();
            /** assume that node wrap that appear to module is not used anymore. */
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

    void Module::simStartCurCycle() {
        /**simulate rtl first*/
        for (int i = 0; i < SP_CNT_REG; i++){
            simStartSubElement(_spRegs[i]);
        }
        simStartSubElement(_userRegs);
        simStartSubElement(_userWires);
        simStartSubElement(_userExpressions);
        simStartSubElement(_userVals);
        simStartSubElement(_userSubModule);

        /**simulate flow block in which node is implicitly invoked*/
        simStartSubElement(_flowBlockBases);

    }

    void Module::simExitCurCycle() {

        /** exit rtl first*/
        for (int i = 0; i < SP_CNT_REG; i++){
            simExitSubElement(_spRegs[i]);
        }
        simExitSubElement(_userRegs);
        simExitSubElement(_userWires);
        simExitSubElement(_userExpressions);
        simExitSubElement(_userVals);
        simExitSubElement(_userSubModule);

        /** exit flowblock first*/
        simExitSubElement(_flowBlockBases);

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