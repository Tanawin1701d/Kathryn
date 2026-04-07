//
// Created by tanawin on 3/12/2566.
//
#include "controller.h"

namespace kathryn{


    /***
     *
     * flow stack can not have lazy block more than one at a time
     * because if there is consequence lazy block the prior one must be delete
     *
     * */


    FlowBlockBase* ModelController::get_top_flow_block_base_ptr() {
        if (_flow_block_stacks[FLOW_ST_BASE_STACK].empty()){
           return nullptr;
        }
        return _flow_block_stacks[FLOW_ST_BASE_STACK].top();
    }

    FlowBlockBase* ModelController::get_top_flow_block_base_ptr(FLOW_STACK_TYPE flow_stack_type){
        assert(flow_stack_type >= 0);
        assert(flow_stack_type < FLOW_ST_CNT);
        if (_flow_block_stacks[flow_stack_type].empty()){
            return nullptr;
        }
        return _flow_block_stacks[flow_stack_type].top();
    }

    void ModelController::pop_flow_block(FlowBlockBase* fb){
        assert(!_flow_block_stacks[FLOW_ST_BASE_STACK].empty());
        assert( _flow_block_stacks[FLOW_ST_BASE_STACK].top() == fb);

        for (int st_idx: fb->get_sel_fb_stack()){
            assert(st_idx < FLOW_ST_CNT);
            assert(!_flow_block_stacks[st_idx].empty());
            assert(_flow_block_stacks[st_idx].top() == fb);
            logMF(fb, "pop_flowBlock " + std::to_string(st_idx));
            _flow_block_stacks[st_idx].pop();
        }
    }

    void ModelController::push_flow_block(FlowBlockBase* fb) {
        assert(fb != nullptr);
        for (int st_idx: fb->get_sel_fb_stack()){
            logMF(fb, "push to stack " + std::to_string(st_idx));
            assert(st_idx < FLOW_ST_CNT);
            _flow_block_stacks[st_idx].push(fb);
        }
    }

    void ModelController::detach_top_flow_block() {

        /**get top of the flow block base and build the hardware*/
        FlowBlockBase* top_fb = get_top_flow_block_base_ptr();
        assert(top_fb != nullptr);
        logMF(top_fb, "on_detach_flowBlock");
        pop_flow_block(top_fb);
        //////topFb->buildHwComponent();


        /**get front node to inject the subblock*/
        FlowBlockBase* front_fb = get_top_flow_block_base_ptr();
        if (!is_top_fb_belong_to_top_module()){
            logMF(top_fb, "addFlowBlock to module");
            Module* parent_mod = get_top_module_ptr();
            parent_mod->addFlowBlock(top_fb);
        }else if (top_fb->get_join_fb_pol() == FLOW_JO_CON_FLOW){
            /**it is consecutive block*/
            logMF(top_fb, "addFlowBlock to be con module");
            front_fb->add_con_flow_block(top_fb);
        }else if (top_fb->get_join_fb_pol() == FLOW_JO_SUB_FLOW){
            /**it is sub block*/
            logMF(top_fb, "addFlowBlock to be sub module");
            front_fb->add_sub_flow_block(top_fb);
        }else if (top_fb->get_join_fb_pol() == FLOW_JO_EXT_FLOW){
            logMF(top_fb, "extract flowblock and give it to basic asm");
            std::vector<AsmNode*> extracted_AsmNode = top_fb->extract();
            for (auto asmNode: extracted_AsmNode){
                front_fb->add_basic_node(asmNode);
            }
            // for (auto basicNode: topFb->getBasicNode()){
            //     frontFb->addElementInFlowBlock(basicNode);
            // }
            front_fb->add_abandon_flow_block(top_fb);
            /***we must delete this due to*/
            ///// delete topFb;
        }else{
            assert(false);
        }

    }

    void ModelController::assign_flow_block_parent(FlowBlockBase* fb){

        /** assign master module*/
        Module* parent_mod = get_target_module_ele().md;
        assert(parent_mod != nullptr);
        fb->setParent(parent_mod);

        /** assign master flow block*/
        if (is_top_fb_belong_to_top_module()){
            FlowBlockBase* top_fb = get_top_flow_block_base_ptr();
            fb->setParent(top_fb);
        }
    }


    bool ModelController::is_all_flow_stack_empty(){
        bool empty_status = true;
        for(const auto & flow_block_stack : _flow_block_stacks){
            empty_status &= flow_block_stack.empty();
        }
        return empty_status;
    }

    bool ModelController::is_flow_stack_empty(FLOW_STACK_TYPE flow_stack_type){

        assert(flow_stack_type >= 0);
        assert(flow_stack_type < FLOW_ST_CNT);

        return _flow_block_stacks[flow_stack_type].empty();
    }


    bool ModelController::is_top_fb_belong_to_top_module(){
        assert(get_top_module_ptr() != nullptr);
        return (!_flow_block_stacks[FLOW_ST_BASE_STACK].empty()) &&
                (_flow_block_stacks[FLOW_ST_BASE_STACK].top()->getModuleParent() == get_top_module_ptr());

    }

    void ModelController::try_purify_flow_stack() {
        FlowBlockBase* fb = get_top_flow_block_base_ptr();
        if (fb == nullptr){return;}
        if (fb->is_lazy_delete()){
            logMF(fb, "strong purify stack");
            detach_top_flow_block();
        }
    }

    void ModelController::on_attach_flowBlock(FlowBlockBase *fb) {
        /*** check purify flow stack*/
        logMF(fb, "on_attach_flowBlock");
        assert(fb != nullptr);
        if (fb->get_purify_req()){
            logMF(fb, "try purify stack");
            try_purify_flow_stack();
        }

        assign_flow_block_parent(fb);
        fb->build_inherit_name();
        /*** add to stack*/
        push_flow_block(fb);
    }

    void ModelController::on_detach_flowBlock(FlowBlockBase* fb) {

        /** to clean lazy delete of the flow block
         * due to last lazy delete pupose
         * */
        auto top_fb = get_top_flow_block_base_ptr();
        if (fb != top_fb){
            ////// it must be lazy delete block inside that it is not deleted yet
            assert(top_fb->is_lazy_delete());
            /////// delete it now
            try_purify_flow_stack();
        }

        /** get our block detach*/
        auto actual_detach_block = get_top_flow_block_base_ptr();

        /** if current flowblock is lazy delete do not detach it*/
        assert(actual_detach_block == fb);
        if (fb->is_lazy_delete()){
            return;
        }else{
            detach_top_flow_block();
        }

    }

    void ModelController::on_attachAndDetach_intrSignal(INT_TYPE intType, Operable* sig) {
        assert(sig != nullptr);
        try_purify_flow_stack();
        mf_assert(is_top_fb_belong_to_top_module(), "There is no flow block to add intr signal");
        auto topFb = get_top_flow_block_base_ptr();
        topFb->add_intr_signal(intType, sig);

    }

    FLOW_BLOCK_TYPE ModelController::get_top_pattern_flow_block_type(){

        bool top_pattern_fb_belong_to_top_module =
                  (!_flow_block_stacks[FLOW_ST_PATTERN_STACK].empty())
                && (_flow_block_stacks[FLOW_ST_PATTERN_STACK].top()->getModuleParent() == get_top_module_ptr()
                );

        if (top_pattern_fb_belong_to_top_module){
            FlowBlockBase* fb = _flow_block_stacks[FLOW_ST_PATTERN_STACK].top();
            assert(fb != nullptr);
            FLOW_BLOCK_TYPE fb_type = fb->get_flow_type();
            assert(fb_type >= SEQUENTIAL && fb_type <= PARALLEL_AUTO_SYNC);
            return fb_type;
        }
        return DUMMY_BLOCK;
    }

    bool ModelController::is_top_of_stack_belong_to_the_same_module(FLOW_STACK_TYPE a,
                                                                    FLOW_STACK_TYPE b){

        if (is_flow_stack_empty(a) ||
            is_flow_stack_empty(b)){return false;}


        FlowBlockBase* flow_block_a = get_top_flow_block_base_ptr(a);
        FlowBlockBase* flow_block_b = get_top_flow_block_base_ptr(b);
        assert(flow_block_a != nullptr && flow_block_b != nullptr);

        Module* parent_a = flow_block_a->getModuleParent();
        Module* parent_b = flow_block_b->getModuleParent();

        assert(parent_a != nullptr &&  parent_b != nullptr);
        return parent_a == parent_b;

    }

}