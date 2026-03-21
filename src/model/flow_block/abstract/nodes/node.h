//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_NODE_H
#define KATHRYN_NODE_H

#include "utility"
#include "vector"
#include "memory"
#include "queue"
#include "map"

#include "model/controller/clock_mode.h"
#include "model/hw_component/abstract/operation.h"
#include "model/hw_component/expression/expression.h"
#include "model/hw_component/register/register.h"
#include "model/hw_component/abstract/assignable.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/sp_reg/sync_reg.h"
#include "model/debugger/model_debugger.h"
#include "sim/model_sim_engine/flow_block/flow_base_sim.h"


namespace kathryn {

    struct NodeWrap;

    enum NODE_TYPE{
        ASM_NODE,
        STATE_NODE,
        SYN_NODE,
        PSEUDO_NODE,
        DUMMY_NODE,
        OPR_NODE,
        START_NODE,
        WAITCOND_NODE,
        WAITCYCLE_NODE,
        COUNTER_NODE,
        NODE_TYPE_CNT
    };

    constexpr int NODE_CYCLE_USED_UNKNOWN = -1;


    /***
     * NODE operation requirement
     * 1. declare Node
     * 2. add condition (optional)
     * 3. add depend_node (at least one node)
     * 4. set join_depend_node_op (required)
     * 5. assign (required)
     * 1-4 can be set independently order, but 5 must declare at last time.
     * **/

    std::string NT_to_string(NODE_TYPE nt);

    struct NodeSrcEdge{
        Node*     depend_node = nullptr;
        Operable* condition  = nullptr;
    };

    struct Node : public ModelDebuggable
                {
        NODE_TYPE                node_type  = NODE_TYPE_CNT;
        std::vector<NodeSrcEdge> node_srcs;
        std::string              ident_name = "NODE_UNNAME";

        Node*                    int_reset  = nullptr;
        Node*                    hold_node  = nullptr;

        CLOCK_MODE               clk_mode = CM_CLK_FREE; //// indicate the clk sensitivity type
        /// for asm node it should be assigned since the was built
        /// for other flowblock should assign it

        /** simulate support register*/
        std::vector<CtrlFlowRegBase*> related_cycle_consume_reg;

        Node(Node& rhs) = delete;

        explicit Node(NODE_TYPE nt):
                node_type(nt){};

        ~Node(){};
        NODE_TYPE get_node_type() const{
            return node_type;
        }

        static void add_logic(Operable* &des_logic, Operable *opr1, LOGIC_OP op) {
            assert(op == BITWISE_AND || op == BITWISE_OR);
            assert(opr1 != nullptr);
            if (des_logic == nullptr) {
                des_logic = opr1;
                return;
            }

            if (op == BITWISE_AND) {
                des_logic = &((*des_logic) & (*opr1));
            } else if (op == BITWISE_OR) {
                des_logic = &((*des_logic) | (*opr1));
            }
        }

        static Operable* add_logic_with_output(Operable* opr1, Operable* opr2, LOGIC_OP op){
            assert(op == BITWISE_AND || op == BITWISE_OR);

            if ( (opr1 == nullptr) && (opr2 == nullptr)){
                return nullptr;
            }

            if(opr1 == nullptr){
                return opr2;
            }else if(opr2 == nullptr){
                return opr1;
            }else if(op == BITWISE_AND) {
                return &((*opr1) & (*opr2));
            }else if(op == BITWISE_OR) {
                return &((*opr1) | (*opr2));
            }else{
                assert(false);
            }
        }

        /** add depend_node for assignment*/
        void add_depend_node(Node* src_node, Operable* cond) {
            assert(src_node != nullptr);
            node_srcs.push_back({src_node, cond});
        }
        /** add to related_cycle_consume_reg which is used to identify register that related to cycle usage*/
        void add_cycle_related_reg(CtrlFlowRegBase* ctrl_reg){
            assert(ctrl_reg != nullptr);
            related_cycle_consume_reg.push_back(ctrl_reg);
        }

        std::vector<CtrlFlowRegBase*>& get_cycle_related_reg(){
            return related_cycle_consume_reg;
        }

        std::vector<NodeSrcEdge>& get_depend_nodes() {return node_srcs;}

        /**
         * clock
         *
         */
        void       set_clock_mode(CLOCK_MODE cm){ clk_mode = cm;}
        CLOCK_MODE get_clock_mode() const{ return clk_mode;}

        /**
         * interrupt handler
         */
        ///// interrupt reset
        void set_interrupt_reset(Node* rst){
            assert(rst != nullptr);
            int_reset = rst;
        }

        bool is_threre_int_reset  () const{ return int_reset != nullptr; }

        Node*get_interrupt_reset_ptr() const{ return int_reset; }

        Operable* bind_with_rst_out_put_if_reset(Operable* raw_exit){
            assert(raw_exit != nullptr);
            assert(raw_exit->get_operable_slice().get_size() == 1);
            if (is_threre_int_reset()){
                return &( (*raw_exit) & (~(*get_interrupt_reset_ptr()->get_exit_opr_ptr())) );
            }
            return raw_exit;
        }
        ///// hold signal
        void set_hold(Node* hn){ /// hold node
            assert(hn != nullptr);
            assert(hold_node == nullptr); /// we do not allow override the dsame node
            hold_node = hn;
        }

        bool  is_there_hold()  const { return hold_node != nullptr; }

        Node*get_hold_node_ptr() const { return hold_node; }

        Operable* bind_with_hold_if_hold(Operable* raw_exit){
            assert(raw_exit != nullptr);
            assert(raw_exit->get_operable_slice().get_size() == 1);

            if (is_there_hold()){
                return &((*raw_exit) & (~(*get_hold_node_ptr()->get_exit_opr_ptr())));
            }
            return raw_exit;
        }



        /**
         * function that allow sp node custom their behavior
         * **/
        /** unset event when state is raised there must be condition that bring this down*/
        virtual void      make_unset_state_event(){assert(false);}
        virtual void      make_user_reset_event(){assert(false);}
        /** provided src state data*/
        virtual Operable*get_exit_opr_ptr(){ return nullptr; };
        virtual Operable*get_state_operating_ptr(){return nullptr;}
        /** assign value with proper condition*/
        virtual void      assign() = 0; /** please make sure that makeunset_state is called*/
        virtual void      dry_assign(){assert(false);};
        /** cycle that is use in this node*/
        virtual int       get_cycle_used() = 0;
        /** is Stateful node (reffer to node that consume at least 1 cycle from machine)*/
        virtual bool      is_state_full_node(){ return true; }

        /** get debugger value*/
        std::string get_md_ident_val() override{
            std::string ret = NT_to_string(node_type) + " @ " + std::to_string((ull)this);
            return ret;
        }
        void add_md_log(MdLogVal* md_log_val) override{
            md_log_val->add_val("[Node] " + get_md_ident_val() +  "have node dep");
            for (auto dep_src : node_srcs){
                md_log_val->add_val(dep_src.depend_node->get_md_ident_val());
            }
        }
        /** internal value identifier for debugging purpose*/
        void set_internal_ident(std::string ident_val){
            ident_name = std::move(ident_val);
        }



    };

}

#endif //KATHRYN_NODE_H
