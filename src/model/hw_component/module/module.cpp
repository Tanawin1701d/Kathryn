//
// Created by tanawin on 30/11/2566.
//

#include "module.h"

#include "util/term_color/term_color.h"


#include "model/controller/controller.h"


namespace kathryn{


    Module::Module(bool init_comp):
            Identifiable(TYPE_MODULE),
            _moduleSimEngine(new ModuleSimEngine(this)){
        if (init_comp)
            com_init();
    }

    Module::~Module() {
        ///delete_sub_element(_stateRegs);
        for(auto& _spReg: _spRegs){
            delete_sub_element(_spReg);
        }
        delete_sub_element(_flowBlockBases);
        /** delete user element*/
        delete_sub_element(_userRegs);
        delete_sub_element(_userWires);
        delete_sub_element(_userExpressions);
        delete_sub_element(_userVals);
        delete_sub_element(_userPmVals);
        delete_sub_element(_userMemBlks);
        delete_sub_element(_userNests);
        delete_sub_element(_userSubModules);
        delete_sub_element(_userBoxs);

        delete _moduleSimEngine;
        delete _moduleGenEngine;

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
    void Module::add_sp_reg(Reg* reg, SP_REG_TYPE sp_reg_type){
        assert(reg != nullptr); /// can't be nullptr
        assert(sp_reg_type < SP_CNT_REG);
        _spRegs[sp_reg_type].push_back(reg);
    }

    void Module::add_flow_block(FlowBlockBase* fb) {
        assert(fb != nullptr);
        _flowBlockBases.push_back(fb);
    }

    void Module::add_user_reg(Reg* reg) {
        assert(reg != nullptr);
        _userRegs.push_back(reg);
    }

    void Module::add_user_wires(Wire* wire) {
        assert(wire != nullptr);
        _userWires.push_back(wire);
    }

    void Module::add_user_expression(expression* expr) {
        assert(expr != nullptr);
        _userExpressions.push_back(expr);
    }

    void Module::add_user_val(Val* val) {
        assert(val != nullptr);
        _userVals.push_back(val);
    }

    void Module::add_user_pm_val(PmVal* pm_val){
        assert(pm_val != nullptr);
        _userPmVals.push_back(pm_val);
    }


    void Module::add_user_mem_blk(MemBlock* mem_block) {
        assert(mem_block != nullptr);
        _userMemBlks.push_back(mem_block);
        /** manual*/
    }

    void Module::add_user_nest(nest* nst){
        assert(nst != nullptr);
        _userNests.push_back(nst);
    }


    void Module::add_user_sub_module(Module* smd){
        assert(smd != nullptr);
        _userSubModules.push_back(smd);
    }

    void Module::add_user_box(Box* box){
        assert(box != nullptr);
        _userBoxs.push_back(box);
    }

    void Module::add_user_itf(ModelInterface* itf){
        assert(itf != nullptr);
        _userItfs.push_back(itf);
    }

    bool Module::is_top_module(){
        return _isTopModule;
    }

    void Module::set_top_module(){
        if (_parent != nullptr){
            std::cout << TC_YELLOW <<"warning the top module have other host module" << TC_DEF << std::endl;
        }
        _isTopModule = true;
    }

    std::vector<Wire*>
    Module::get_user_wires_by_marker(WIRE_MARKER_TYPE wmt){
        std::vector<Wire*> result;
        for (Wire* wire: get_user_wires()){
            assert(wire != nullptr);
            if (wire->get_marker() == wmt){
                result.push_back(wire);
            }
        }
        return result;
    }


    void Module::build_all(){
        /**declare to model controller that this module is initialize*/
        flow();
        for (ModelInterface* itf: get_itfs()){
            itf->build_logic_base();
        }
        /** clear everything left in flowblock stack*/
        ctrl->try_purify_flow_stack();
        assert(ctrl->is_all_flow_stack_empty());

        /**this ensure that submodule in init component and all is ready to build flow*/
        /** build sub module first*/
        for (auto sub_md: _userSubModules){
            ctrl->on_module_init_designFlow(sub_md);
            ctrl->on_module_final(sub_md);
        }
        build_flow();

        /**
         *
         * please note that you cant change order of flow and sub model init design flow
         * */

    }

    void Module::build_flow(){

        std::vector<NodeWrap*> front_node_wrap;

        if (get_global_id() == 8){
            std::cout << "start build flow of a" << std::endl;
        }

        for (auto fb: _flowBlockBases){
            assert(fb != nullptr);
            switch (fb->get_join_fb_pol()) {

                case FLOW_JO_SUB_FLOW:
                    /**in case it is normal flow block*/
                    fb->build_hw_master();
                    front_node_wrap.push_back(fb->sumarize_block());
                    break;
                case FLOW_JO_CON_FLOW:
                    mf_assert(false, "detect con bare block iteration");
                    break;
            case FLOW_JO_EXT_FLOW:
                    /**in case it is extract need flow block*/
                    std::vector<AsmNode*> extracted_asm_node = fb->extract();
                    for (auto node: extracted_asm_node){
                        node->dry_assign();
                    }
                    break;
            }

        }
        for (auto nw: front_node_wrap){
            /** we will have start wire node to start node*/
            nw->add_depend_node_to_all_node(start_node);
            nw->assign_all_node();
            /** assume that node wrap that appear to module is not used anymore. */
        }

        /**check short circuit*/
        for(auto expr: _userExpressions){
            expr->start_check_short_circuit();
        }
        for(auto wire: _userWires){
            wire->start_check_short_circuit();
        }

    }


    std::string
    Module::get_md_describe(){

        for (auto fb: _flowBlockBases){
            //log_md(fb->get_md_ident_val(), fb->get_md_describe());
        }

        return "";

    }

    void Module::create_module_gen(){
        _moduleGenEngine = new ModuleGen(this);
    }


    void Module::add_md_log(MdLogVal *md_log_val) {
        md_log_val->add_val("[ " + get_md_ident_val() + " ]");
        for (auto sb : _flowBlockBases){
            auto sub_log = md_log_val->make_new_sub_val();
            sb->add_md_log(sub_log);
        }
    }

}