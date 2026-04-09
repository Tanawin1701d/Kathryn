//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_FLOWBLOCK_BASE_H
#define KATHRYN_FLOWBLOCK_BASE_H


#define intrReset( expr  )   kathryn_block->add_intr_signal(INT_RESET, &expr);
#define intrStart( expr  )   kathryn_block->add_intr_signal(INT_START, &expr);
#define holdBlk( expr  )     kathryn_block->add_hold_signal(&expr);
#define intrRstAndStart( expr ) intrReset(expr) intrStart(expr)
#define exposeBlk( exVar )   exVar = kathryn_block;
#define track( name  )       kathryn_block->set_zep_track_name(#name);
#define strack( name )       kathryn_block->set_zep_track_name(name);
#define markJoinMaster       kathryn_block->set_join_master();

#include<memory>
#include<vector>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/abstract/operation.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodeWrap.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"
#include "model/flowBlock/abstract/nodes/logicNode.h"

#include "sim/modelSimEngine/flowBlock/flowBaseSim.h"
#include "sim/modelSimEngine/flowBlock/flowBlockProber.h"


#include "flowIdentifiable.h"
#include "flowBlockRegister.h"

#include "logicHelper.h"
#include "model/flowBlock/cond/zifClassAsm.h"


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
        std::vector<FLOW_STACK_TYPE> sel_flow_stack; //////// which stack for push/pop
        FLOW_BLOCK_JOIN_POLICY       join_policy; ////// how to join with other block
        bool                         req_purify = false;
    };

    extern int nextFbIdx;

    ////// extract the block into node
    struct NodeExtractable{
        virtual ~NodeExtractable() = default;

        virtual std::vector<AsmNode*> extract() { assert(false);}

    };

    class FlowBlockBase: public NodeExtractable,
                         public FlowIdentifiable,
                         public ModelDebuggable,
                         public FlowSimEngineInterface
                         {
    protected:

        struct sortEle{
            FlowBlockBase* fb;
            int order = -1;

            bool operator< (const sortEle& rhs) const{
                return order < rhs.order;
            }
        };

        int nextInputOrder = 0;
        /** 
         * flow element
         */
        std::vector<FlowBlockBase*> _sub_blocks;
        std::vector<int>            _sub_blocks_order; //// input order in this block
        std::vector<FlowBlockBase*> _con_blocks;
        std::vector<int>            _con_blocks_order; //// input order in this block
        std::vector<Node*>          _basic_nodes;
        std::vector<int>            _basic_nodes_order; //// input order in this block
        std::vector<Node*>          _sys_nodes;

        std::vector<FlowBlockBase*> _abandoned_blocks;  /// the flow block that have been extracted and push to this block
        std::vector<Node*>          _abandoned_nodes;    /// the node that have been extracted and push to join collective node

        /** 
         * interrupt element
         */
        ///  basic interrupt signals
        std::vector<Operable*>        _int_signals[INT_CNT];
        OprNode*                      _int_nodes  [INT_CNT]; //// the sum of allnode
        /// basic  hold signals
        /// the hold signal supposed to hold the state without execute it
        std::vector<Operable*>        _hold_signals;
        OprNode*                      _hold_node = nullptr;
        
        /** 
         * CLOCK MODE
         */
        CLOCK_MODE                    _clk_mode;
        
        /**
         * Flow Block <-> ctrl data/metadata
         */
        /// status of node
        FLOW_BLOCK_TYPE             _type;
        ModelController*            _ctrl                  = nullptr;
        bool                        _lazy_deleted_required = false;
        int                         _fb_id;
        /// controller interactive element
        FB_CTRL_COM_META            _fb_ctrl_com_meta;
        /// for exit management
        bool                        _are_there_force_exit = false;
        PseudoNode*                 _force_exit_node     = nullptr;
        //// flow block sim engine
        FlowBaseSimEngine*          _flow_sim_engine     = nullptr;


        /// generate implicit subblock typically used with if and while block
        FlowBlockBase* gen_implicit_sub_blk(FLOW_BLOCK_TYPE defaultType);
        /// generate sum of force exit note (the global variable)
        void gen_sum_force_exit_node(std::vector<NodeWrap*>& nws);
        /// interrupt node
        void fill_intr_rst_signal_to_child(); /// use for pass the data when build
        void gen_intr_node                ();
        bool is_there_intr_start          ();
        bool is_there_intr_rst            ();
        /// holding system
        void fill_hold_signal_to_child();
        void gen_hold_node            ();
        bool is_there_hold            ();
        /// clock mode
        void set_clock_mode      (CLOCK_MODE mode);
        CLOCK_MODE get_clock_mode() const{ return _clk_mode; }

        ///
        Operable*      purify_condition          (Operable* raw_opr);
        FlowBlockBase* scan_master_join_sub_block();

    public:
        explicit FlowBlockBase(FLOW_BLOCK_TYPE  type,
                               FB_CTRL_COM_META fb_ctrl_com_meta);
        virtual ~FlowBlockBase();

        Operable* gen_intr_sum_signal(bool is_and_cond, INT_TYPE intr_type); /// it pool all condition to single signal

        FlowBaseSimEngine* get_sim_engine() override{
            return _flow_sim_engine;
        }

        /**
         * entrance to make controller interact with
         * */

        /// when basic behavior describe in flow block
        virtual void add_basic_node(Node* node){
            assert(node != nullptr);
            _basic_nodes.push_back(node);
            _basic_nodes_order.push_back(nextInputOrder++);
        }

        /// system node is the node used to monitor by hybrid profiler
        virtual void add_sys_node(Node* node){
            assert(node != nullptr);
            _sys_nodes.push_back(node);
        }

        /// when inside complex element such as sub flow block is finish, user must add here
        virtual void add_sub_flow_block(FlowBlockBase* sub_block){
            assert(sub_block != nullptr);
            _sub_blocks.push_back(sub_block);
            _sub_blocks_order.push_back(nextInputOrder++);
        };
        /// add sub con block as consecutive block
        virtual void add_con_flow_block(FlowBlockBase* con_block){
            assert(con_block != nullptr);
            _con_blocks.push_back(con_block);
            _con_blocks_order.push_back(nextInputOrder++);
        }

        virtual void add_abandon_flow_block(FlowBlockBase* abandon_block){
            assert(abandon_block != nullptr);
            _abandoned_blocks.push_back(abandon_block);
        }

        virtual void add_abandon_node(Node* abandon_node){
            assert(abandon_node != nullptr);
            _abandoned_nodes.push_back(abandon_node);
        }

        /// interrupt signal
        virtual void add_intr_signal(INT_TYPE type, Operable* signal){
            assert(signal != nullptr);
            assert(type < INT_CNT);
            assert(signal->getOperableSlice().getSize() == 1);
            _int_signals[type].push_back(signal);
        }

        void fill_intr_reset_to_node_if_there(Node* nd){
            if (_int_nodes[INT_RESET] != nullptr){
                nd->set_interrupt_reset(_int_nodes[INT_RESET]);
            }
        }

        /// hold entrance
        virtual void add_hold_signal(Operable* signal){
            assert(signal != nullptr);
            _hold_signals.push_back(signal);
        }

        void fill_hold_to_node_if_there(Node* nd){
            if (_hold_node != nullptr){
                nd->set_hold(_hold_node);
            }
        }

        std::vector<sortEle> sort_sub_and_con_fb_inorder();
        void                 override_clock_mode_in_all_asm_nodes();
        /**
         * For custom block
         * */
        /// when everything is finish call this to get sumarisation
        virtual NodeWrap* sumarize_block() = 0;
        /// communicator to controller
        virtual void on_attach_block    () = 0; /// it is supposed to acknowledge controller whether this block is declared
        virtual void on_detach_block    () = 0;
        /// for module controller build node and other elements
        virtual void build_hw_master       ();
        virtual void build_sub_hw_component();
        virtual void build_hw_component    () = 0;
        /// getter/setter
        FLOW_BLOCK_TYPE get_flow_type()     const{ return _type; }
        int             get_flow_block_id() const{ return _fb_id; }

        std::vector<Node*>&
        get_basic_nodes_ref(){ return _basic_nodes; }

        std::vector<FlowBlockBase*>&
        get_sub_blocks_ref(){ return _sub_blocks; }

        std::vector<FlowBlockBase*>&
        get_con_blocks_ref(){ return _con_blocks; }

        std::vector<Node*>&
        get_sys_nodes_ref(){ return _sys_nodes; }

        /** lazy delete is the variable that tell controller whether
         * block should be pop from building stack when purifier is done
         * not when block is detach. Usually, It is used in if block
         * */
        bool is_lazy_delete() const{ return _lazy_deleted_required; }
        void set_lazy_delete(){ _lazy_deleted_required = true; }
        void unset_lazy_delete(){ _lazy_deleted_required = false; }
        /// controller communication
        [[nodiscard]]
        std::vector<FLOW_STACK_TYPE> get_sel_fb_stack() const{ return _fb_ctrl_com_meta.sel_flow_stack; }

        [[nodiscard]]
        FLOW_BLOCK_JOIN_POLICY get_join_fb_pol() const{ return _fb_ctrl_com_meta.join_policy; }

        [[nodiscard]]
        bool get_purify_req() const{ return _fb_ctrl_com_meta.req_purify; }

        /// debug method
        std::string get_md_describe_recur(){
            std::string ret = "----------- sub Block --------\n";
            for (auto sb : _sub_blocks){
                ret += sb->get_md_describe();
                ret += "\n";
            }
            return ret;
        }

        void add_md_log_recur(MdLogVal* md_log_val){
            if (_sub_blocks.empty())
                return;
            md_log_val->addVal("-----sub block------");
            for (auto sb : _sub_blocks){
                auto sub_struct = md_log_val->makeNewSubVal();
                sb->add_md_log(sub_struct);
            }
        }

        [[nodiscard]] std::string get_md_ident_val() override{
            return FBT_to_string(get_flow_type()) + "_blockId_" + std::to_string(_fb_id);
        }

    };
}

#endif //KATHRYN_FLOWBLOCK_BASE_H
