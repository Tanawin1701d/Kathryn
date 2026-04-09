//
// Created by tanawin on 14/2/2026.
//

#ifndef MODEL_FLOWBLOCK_ABSTRACT_NODES_CNTNODE_H
#define MODEL_FLOWBLOCK_ABSTRACT_NODES_CNTNODE_H

#include "node.h"
#include "model/flowBlock/abstract/spReg/cntReg.h"

namespace kathryn{

    struct CounterNode : Node{
        CounterReg* _counter = nullptr;

        explicit CounterNode(int lastLoopCnt, CLOCK_MODE clockMode):
                Node(COUNTER_NODE){

            assert(lastLoopCnt > 0);
             _counter = new CounterReg(lastLoopCnt);
            add_cycle_related_reg(_counter);
            set_clock_mode(clockMode);
        }

        void make_unset_state_event() override{
            assert(false);
        }

        void make_user_reset_event() override{
            if(is_threre_int_reset()){
                _counter->make_user_rst_event(_int_reset->get_exit_opr_ptr(), get_clock_mode());
            }
        }

        void make_inc_counter_event(Node* inc_node){
            assert(inc_node != nullptr);
            _counter->make_inc_event(inc_node->get_exit_opr_ptr(), get_clock_mode());
        }

        Operable* get_exit_opr_ptr() override{
            assert(_counter != nullptr);
            return _counter->generate_end_expr();
        }

        Operable* get_counter(){return _counter;}

        void assign() override{
            assert(_counter!= nullptr);
            /**normal start event*/
            for(auto node_src: _node_srcs){
                _counter->add_depend_state(node_src.depend_node->get_exit_opr_ptr(),
                                         node_src.condition, get_clock_mode());
            }
            /** unset event*/
            make_user_reset_event();
            _counter->setVarName(_ident_name);
        }

        int get_cycle_used() override{
            assert(_counter != nullptr);
            ///// hold should not be considered here
            return _counter->get_loop_cnt();
        }

    };

}

#endif //MODEL_FLOWBLOCK_ABSTRACT_NODES_CNTNODE_H
