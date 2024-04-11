//
// Created by tanawin on 30/11/2566.
//

#include "module.h"

#include "model/controller/controller.h"


namespace kathryn{


    Module::Module(bool initComp):
            Identifiable(TYPE_MODULE),
            HwCompControllerItf(),
            ModuleSimEngine(),
            _mdStage(MODEL_UNINIT)
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
        deleteSubElement(_bareNodes);
        /** delete user element*/
        deleteSubElement(_userRegs);
        deleteSubElement(_userWires);
        deleteSubElement(_userExpressions);
        deleteSubElement(_userVals);
        deleteSubElement(_userMemBlks);
        deleteSubElement(_userNests);
        deleteSubElement(_userSubModules);
        deleteSubElement(_userBoxs);

    }

    void Module::com_init() {
        ctrl->on_module_init_components(this);
        /**post finalize component must be handle when object is buit finish*/
    }

    void Module::com_final() {
        /** invoke controller to design end component init*/
        ctrl->on_module_end_init_components(this);
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

    void Module::addNode(Node* node) {
        assert(node != nullptr);
        _bareNodes.push_back(node);
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
        /** manual*/
    }

    void Module::addUserNest(nest* nst){
        assert(nst != nullptr);
        _userNests.push_back(nst);
    }


    void Module::addUserSubModule(Module* smd){
        assert(smd != nullptr);
        _userSubModules.push_back(smd);
    }

    void Module::addUserBox(Box* box){
        assert(box != nullptr);
        _userBoxs.push_back(box);
    }

    void Module::buildAll(){
        /**declare to model controller that this module is initialize*/
        flow();
        buildFlow();
        /**this ensure that submodule in init component and all is ready to build flow*/
        /** build sub module first*/
        for (auto subMd: _userSubModules){
            ctrl->on_module_init_designFlow(subMd);
            ctrl->on_module_final(subMd);
        }

        /**
         *
         * please note that you cant change order of flow and sub model init design flow
         * */

    }

    void Module::buildFlow(){

        ctrl->tryPurifyFlowStack();
        assert(ctrl->isAllFlowStackEmpty());

        std::vector<NodeWrap*> frontNodeWrap;

        for (auto fb: _flowBlockBases){
            assert(fb != nullptr);
            switch (fb->getJoinFbPol()) {

                case FLOW_JO_SUB_FLOW:
                    /**in case it is normal flow block*/
                    fb->buildHwComponent();
                    frontNodeWrap.push_back(fb->sumarizeBlock());
                    break;
                case FLOW_JO_CON_FLOW:
                    mfAssert(false, "detect con bare block iteration");
                    break;
                case FLOW_JO_EXT_FLOW:
                    /**in case it is extract need flow block*/
                    for (auto node: fb->getBasicNode()){
                        node->dryAssign();
                    }
                    break;
            }

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
        beforePrepareSimSubElement_RTL_only(_userNests, vcdWriter);
                ////// memblock doesn't have before prepare sim subelement
        /**flow block prepare sim*/
        beforePrepareSimSubElement_FB_only(_flowBlockBases, flowColEle->populateSubEle());
        /**COMPLEX BEFORE PREPARE SUB SIM*/
        for(auto modulePtr : _userSubModules){
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
        prepareSimSubElement(_userNests);
        /**COMPLEX PREPARE SUB SIM*/
        prepareSimSubElement(_userSubModules);


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
        simStartCurSubElement(_userNests);
        /**COMPLEX SUB SIM*/
        simStartCurSubElement(_userSubModules);
        /**for now we are sure that the other is simulated*/
        simStartCurSubElement(_flowBlockBases);

        /**simulate flow block in which node is implicitly invoked*/
        setCurValSimStatus();
    }

    void Module::simStartNextCycle() {
        /**RTL SIM(SP)*/
        for (int i = 0; i < SP_CNT_REG; i++){
            simStartNextSubElement(_spRegs[i]);
        }
        /**RTL SIM(USER)*/
        simStartNextSubElement(_userRegs);
        simStartNextSubElement(_userMemBlks);
            /**for wire expresssion and nest is not need to implement*/
        /**COMPLEX SUB SIM*/
        simStartNextSubElement(_userSubModules);
            /**for flow block no next cycle to determine*/

        setNextValSimStatus();
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
        curCollectData(_userNests);
            /**memblock doesn't need to collect data*/
        /**COMPLEX SUB SIM*/
        curCollectData(_userSubModules);
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
        simExitSubElement(_userNests);
        /**COMPLEX SUB SIM*/
        simExitSubElement(_userSubModules);
        simExitSubElement(_flowBlockBases);

        /** exit flowblock first*/
        isCurCycleSimulated  = false;
        isNextCycleSimulated = false;
    }





    /** sub element interface*/
    template<typename T>
    void Module::beforePrepareSimSubElement_RTL_only(std::vector<T*>& subEleVec,
                                                     VcdWriter* writer){

        /****/

        for (auto ele: subEleVec){
            assert(ele != nullptr);
            LogicSimEngine*  logicSimEngine = (LogicSimEngine*)(ele->getSimEngine());

            logicSimEngine->beforePrepareSim(
                    {true, /// for now
                             ele->concat_inheritName()+ "_" + ele->getVarName(),
                             writer
                            });
            /***sort assignable to support sim*/
            ele->sortUpEventByPriority();
        }
    }

    void Module::beforePrepareSimSubElement_FB_only(std::vector<FlowBlockBase*>& subEleVec, FlowColEle* flowColEle){
        assert(flowColEle != nullptr);
        /**set writer for this ele*/
        flowColEle->localName = concat_inheritName();
        flowColEle->freq = 0;
        /**set writer for sub flow block*/
        for (auto* fb: subEleVec){
            fb->beforePrepareSim({
                                         flowColEle->populateSubEle()
                                 });
        }

    }
    template<typename T>
    void Module::prepareSimSubElement(std::vector<T*>& subEleVec){
        for (auto ele: subEleVec){
            assert(ele != nullptr);
            ele->getSimEngine()->prepareSim();
        }
    }
    template<typename T>
    void Module::simStartCurSubElement(std::vector<T*>& subEleVec){
        for (auto ele: subEleVec){
            assert(ele != nullptr);
            ele->getSimEngine()->simStartCurCycle();
        }
    }
    template<typename T>
    void Module::simStartNextSubElement(std::vector<T*>& subEleVec){
        for(auto ele: subEleVec){
            assert(ele != nullptr);
            ele->getSimEngine()->simStartNextCycle();
        }
    }
    template<typename T>
    void Module::curCollectData(std::vector<T*>& subEleVec){
        for (auto ele: subEleVec){
            ele->getSimEngine()->curCycleCollectData();
        }
    }
    template<typename T>
    void Module::simExitSubElement(std::vector<T*>& subEleVec){
        for (auto ele: subEleVec){
            ele->getSimEngine()->simExitCurCycle();
        }
    }

}