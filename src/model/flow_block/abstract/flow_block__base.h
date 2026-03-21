//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_FLOWBLOCK_BASE_H
#define KATHRYN_FLOWBLOCK_BASE_H


#define intr_reset( expr  )   kathryn_block->add_int_signal(INT_RESET, &expr);
#define intr_start( expr  )   kathryn_block->add_int_signal(INT_START, &expr);
#define hold_blk( expr  )     kathryn_block->add_hold_signal(&expr);
#define intr_rst_and_start( expr ) intr_reset(expr) intr_start(expr)
#define expose_blk( ex_var )   ex_var = kathryn_block;
#define track( name  )       kathryn_block->set_zep_track_name(#name);
#define strack( name )       kathryn_block->set_zep_track_name(name);
#define mark_join_master       kathryn_block->set_join_master();

#include "memory"
#include "vector"

#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/register/register.h"
#include "model/hw_component/expression/expression.h"
#include "model/hw_component/abstract/operation.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/node_wrap.h"
#include "model/flow_block/abstract/nodes/state_node.h"
#include "model/flow_block/abstract/nodes/logic_node.h"

#include "sim/model_sim_engine/flow_block/flow_base_sim.h"
#include "sim/model_sim_engine/flow_block/flow_block_prober.h"


#include "flow_identifiable.h"
#include "flow_block_register.h"

#include "logic_helper.h"
#include "model/flow_block/cond/zif_class_asm.h"


namespace kathryn {
    /** it is basic node that only have one event at a node */
    class ModelController;

    enum FLOW_STACK_TYPE{
        FLOW_ST_BASE_STACK      = 0,
        FLOW_ST_PATTERN_STACK   = 1,    /**for seq par*/
        FLOW_ST_HEAD_COND_STACK = 2, /**for if only**/
        FLOW_ST_PIP_BLK         = 3,
        FLOW_ST_CNT             = 4
    };

    enum FLOW_BLOCK_JOIN_POLICY{
        FLOW_JO_SUB_FLOW, /**for other block*/
        FLOW_JO_CON_FLOW, /** join flowblock for elif and else*/
        FLOW_JO_EXT_FLOW, /**extract this flow to be an basic node*/
    };

    enum INT_TYPE{
        INT_RESET = 0,
        INT_START = 1,
        INT_CNT = 2,
    };

    struct FB_CTRL_COM_META{
        std::vector<FLOW_STACK_TYPE> _selFlowStack; //////// which stack for push/pop
        FLOW_BLOCK_JOIN_POLICY       _joinPolicy; ////// how to join with other block
        bool                         req_purify = false;
    };

    extern int next_fb_idx;

    ////// extract the block into node
    struct NodeExtractable{
        virtual ~NodeExtractable() = default;

        virtual std::vector<AsmNode*> extract() { assert(false);};

    };

    class FlowBlockBase: public NodeExtractable,
                         public FlowIdentifiable,
                         public ModelDebuggable,
                         public FlowSimEngineInterface
                         {
    protected:

        struct sort_ele{
            FlowBlockBase* fb;
            int order = -1;

            bool operator< (const sort_ele& rhs) const{
                return order < rhs.order;
            }
        };

        int next_input_order = 0;
        /** flow element*/
        std::vector<FlowBlockBase*> _subBlocks;
        std::vector<int>            _subBlocksOrder; //// input order in this block
        std::vector<FlowBlockBase*> _conBlocks;
        std::vector<int>            _conBlocksOrder; //// input order in this block
        std::vector<Node*>          _basicNodes;
        std::vector<int>            _basicNodesOrder; //// input order in this block
        std::vector<Node*>          _sysNodes;

        std::vector<FlowBlockBase*> _abandonedBlocks;  /// the flow block that have been extracted and push to this block
        std::vector<Node*>          _abandonedNodes;    /// the node that have been extracted and push to join collective node

        /**  basic interrupt signals*/
        std::vector<Operable*>        int_signals[INT_CNT];
        OprNode*                      int_nodes  [INT_CNT]; //// the sum of allnode
        /** basic  hold signals*/
        /** the hold signal supposed to hold the state without execute it*/
        std::vector<Operable*>        hold_signals;
        OprNode*                      hold_node = nullptr;
        /** CLOCK MODE*/
        CLOCK_MODE                    clk_mode;





        /** status of node*/
        FLOW_BLOCK_TYPE             _type;
        ModelController*            ctrl = nullptr;
        bool                        lazy_deleted_required = false;
        int                         _fbId;
        /** controller interactive element*/
        FB_CTRL_COM_META            _fbCtrlComMeta;
        /*** for exit management*/
        bool                        _areThereForceExit = false;
        PseudoNode*                 _forceExitNode     = nullptr;
        /*** flow block sim engine*/
        FlowBaseSimEngine*          _flowSimEngine     = nullptr;

        /** generate implicit subblock typically used with if and while block*/
        FlowBlockBase* gen_implicit_sub_blk(FLOW_BLOCK_TYPE default_type);
        /** generate sum of force exit note (the global variable)*/
        void           gen_sum_force_exit_node(std::vector<NodeWrap*>& nws);
        /** interrupt node*/
        void           fill_int_rst_signal_to_child(); //// use for pass the data when build
        void           gen_int_node();
        bool           is_there_int_start();
        bool           is_there_int_rst();
        /** holding system*/
        void           fill_hold_signal_to_child();
        void           gen_hold_node();
        bool           is_there_hold();
        /** clock mode*/
        void           set_clock_mode(CLOCK_MODE mode);
        CLOCK_MODE     get_clock_mode() const {return clk_mode;}

        ///////////////////////////////////////
        Operable*      purify_condition(Operable* raw_opr);
        FlowBlockBase* scan_master_join_sub_block();
    public:
        explicit       FlowBlockBase(FLOW_BLOCK_TYPE  type,
                                     FB_CTRL_COM_META fb_ctrl_com_meta);
        virtual        ~FlowBlockBase();

        Operable*      gen_int_sum_signal(bool is_and_cond, INT_TYPE intr_type); //// it pool all condition to single signal

        FlowBaseSimEngine* get_sim_engine_ptr() override{
                return _flowSimEngine;
        }
        /**
         * entrance to make controller interact with
         * */
        /** when basic behavior describe in flow block*/
        virtual void add_element_in_flow_block(Node* node){
            assert(node != nullptr);
            _basicNodes.push_back(node);
            _basicNodesOrder.push_back(next_input_order++);
        }
        /** system node is the node used to monitor by hybrid profiler*/
        virtual void add_sys_node(Node* node){
            assert(node != nullptr);
            _sysNodes.push_back(node);
        }
        /** when inside complex element such as sub flow block is finish, user must add here*/
        virtual void add_sub_flow_block(FlowBlockBase* sub_block){
            assert(sub_block != nullptr);
            _subBlocks.push_back(sub_block);
            _subBlocksOrder.push_back(next_input_order++);
        };
        /** add sub con block as consecutive block*/
        virtual void add_con_flow_block(FlowBlockBase* con_block){
            assert(con_block != nullptr);
            _conBlocks.push_back(con_block);
            _conBlocksOrder.push_back(next_input_order++);
        }
        virtual void add_abandon_flow_block(FlowBlockBase* abandon_block){
            assert(abandon_block != nullptr);
            _abandonedBlocks.push_back(abandon_block);
        }

        virtual void add_abandon_node(Node* abandon_node){
            assert(abandon_node != nullptr);
            _abandonedNodes.push_back(abandon_node);
        }

        virtual void add_int_signal(INT_TYPE type, Operable* signal){
            assert(signal != nullptr);
            assert(type < INT_CNT);
            assert(signal->get_operable_slice().get_size() == 1);
            int_signals[type].push_back(signal);
        }

        void fill_int_reset_to_node_if_there(Node* nd){
            if (int_nodes[INT_RESET] != nullptr){
                nd->set_interrupt_reset(int_nodes[INT_RESET]);
            }
        }

        /** hold entrance*/
        virtual void add_hold_signal(Operable* signal){
            assert(signal != nullptr);
            hold_signals.push_back(signal);
        }

        void fill_hold_to_node_if_there(Node* nd){
            if (hold_node != nullptr){
                nd->set_hold(hold_node);
            }
        }

        std::vector<sort_ele> sort_sub_and_con_fb_in_order();
        void                 override_clock_mode_in_all_asm_nodes();
        /**
         * For custom block
         * */
        /** when everything is finish call this to get sumarisation*/
        virtual NodeWrap*   sumarize_block() = 0;
        /*** communicator to controller*/
        virtual void        on_attach_block() = 0; //// it is supposed to acknowledge controller whether this block is declared
        virtual void        on_detach_block() = 0;
        /*** for module controller build node and other elements*/
        virtual void        build_hw_master();
        virtual void        build_sub_hw_component();
        virtual void        build_hw_component() = 0;
        ////// getter/setter
        FLOW_BLOCK_TYPE     get_flow_type() const {return _type;}
        int                 get_flow_block_id() const{return _fbId;}
        std::vector<Node*>&
                            get_basic_node(){return _basicNodes;}
        std::vector<FlowBlockBase*>&
                            get_sub_blocks(){return _subBlocks;}
        std::vector<FlowBlockBase*>&
                            get_con_blocks(){return _conBlocks;}
        std::vector<Node*>&
                            get_sys_nodes(){return _sysNodes;}
        /** lazy delete is the variable that tell controller whether
         * block should be pop from building stack when purifier is done
         * not when block is detach. Usually, It is used in if block
         * */
        bool                is_lazy_delete() const{ return lazy_deleted_required; }
        void                set_lazy_delete()     { lazy_deleted_required = true;}
        void                unset_lazy_delete()   {lazy_deleted_required = false;}
        /** controller communication*/
        [[nodiscard]]
        std::vector<FLOW_STACK_TYPE> get_sel_fb_stack() const {return _fbCtrlComMeta._selFlowStack;}
        [[nodiscard]]
        FLOW_BLOCK_JOIN_POLICY       get_join_fb_pol()  const {return _fbCtrlComMeta._joinPolicy;  }
        [[nodiscard]]
        bool                         get_purify_req()  const {return _fbCtrlComMeta.req_purify;    }

        /** debug method*/
        std::string get_md_describe_recur() {
            std::string ret = "----------- sub Block --------\n";
            for (auto sb : _subBlocks){
                ret += sb->get_md_describe();
                ret += "\n";
            }
            return ret;
        }

        void add_md_log_recur(MdLogVal *md_log_val){
            if (_subBlocks.empty())
                return;
            md_log_val->add_val("-----sub block------");
            for (auto sb: _subBlocks){
                auto sub_struct = md_log_val->make_new_sub_val();
                sb->add_md_log(sub_struct);
            }
        }

        [[nodiscard]]std::string get_md_ident_val() override{
            return FBT_to_string(get_flow_type()) + "_blockId_" + std::to_string(_fbId);
        }
    };

}

#endif //KATHRYN_FLOWBLOCK_BASE_H
