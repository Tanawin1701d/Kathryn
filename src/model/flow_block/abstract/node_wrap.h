//
// Created by tanawin on 31/12/2566.
//

#ifndef KATHRYN_NODEWRAP_H
#define KATHRYN_NODEWRAP_H

#include "model/flow_block/abstract/nodes/node.h"

namespace kathryn{

    static const int IN_CONSIST_CYCLE_USED = -1;

    /* use for inner block return to outter block**/
    struct NodeWrap : ModelDebuggable {
    public:

        /** entrance represent UpdateEvent which refers to node that be head of the subblock*/
        /** note that expr_metas must not be here due to the abstract of the system*/
        std::vector<Node*> entrance_nodes;
        /** the exit condition that allow next building block run*/
        Node* exit_node  = nullptr;
        /** force exit Node is the node that indicate exit expression without concerning of flow_block behaviour*/
        Node* force_exit_node = nullptr;
        /** number of cycle required in this subblock*/
        int cycle_used = IN_CONSIST_CYCLE_USED;

        NodeWrap(const NodeWrap& rhs) {
            ///// we don't support copy constructor anymore
            ///// because it may cause error to asignment node formation and hard to debug
            assert(false);
        }

        NodeWrap() = default;

        NodeWrap& operator=(const NodeWrap& rhs) {
            ///// we don't support copy constructor anymore
            ///// because it may cause error to asignment node formation and hard to debug
            assert(false);
        }

        void add_entrace_node(Node *nd) {
            assert(nd != nullptr);
            entrance_nodes.push_back(nd);
        }

        void add_entrace_nodes(const std::vector<Node*>& nds){
            for (auto nd : nds){
                add_entrace_node(nd);
            }
        }

        void add_exit_node(Node* nd) {
            assert(nd != nullptr);
            exit_node = nd;
        }

        void add_force_exit_node(Node* nd){
            assert(nd != nullptr);
            force_exit_node = nd;
        }

        void add_depend_node_to_all_node(Node* st, Operable* condition=nullptr) {
            assert(st != nullptr);
            for (auto node: entrance_nodes) {
                node->add_depend_node(st, condition);
            }
        }

        /** we force node to declare themselves*/
//        void set_all_depend_node_cond(LOGIC_OP op){
//            for (auto node: entrance_nodes){
//                node->set_depend_state_join_op(op);
//            }
//        }

        void assign_all_node() {
            for (auto node: entrance_nodes) {
                node->assign();
            }
        }

        /** copy node pointer to this wrap*/
        /// todo we will make it copy node if need but for now we don't
        void transfer_ent_node_from(NodeWrap *nw) {
            assert(nw != nullptr);
            for (auto node: nw->entrance_nodes) {
                entrance_nodes.push_back(node);
            }
        }

        void delete_nodes_in_wrap() {
            for (auto nd: entrance_nodes) {
                delete nd;
            }
            /**exit node and force exit_node will be not deleted because it is only pointer it will be not clone*/
        }

        Node* get_exit_node () const {
            assert(exit_node != nullptr);
            return exit_node;
        }
        bool  is_there_force_exit_node() const {return force_exit_node != nullptr;}
        Node*get_force_exit_node_ptr() const {return force_exit_node;}

        void set_cycle_used(int cycle){
            assert(cycle == -1 || cycle > 0);
            cycle_used = cycle;
        }

        int get_cycle_used() const{
            assert(cycle_used == -1 || cycle_used > 0 );
            return cycle_used;
        }

        std::string get_md_describe() override{
            std::string ret;
            ret += "has_entrance_node [";
            for (auto entrance_node : entrance_nodes){
                ret += entrance_node->get_md_ident_val();
                ret += ", ";
            }

            ret += "] has exit_node ";
            ret += exit_node->get_md_ident_val();

            if (is_there_force_exit_node()){
                ret += "has force exit Node ";
                ret += force_exit_node->get_md_ident_val();
            }



            ret += " use cycle " + std::to_string(cycle_used);

            return ret;
        }

        std::string get_md_ident_val() override{
            return "[node_wrap @" + std::to_string((ull)this) + " ]";
        }

    };



    /** this struct is used to determine numbers of cycle
     * that is used in multiple subblock
     * if return -1 means can't determine exact value
     * else x > 0 means these sub flow block use x cycle to
     * complete their work equally
     * */

    struct NodeWrapCycleDet{
        std::vector<int> sampling_vec;

        void add_to_det(std::vector<Node*>& nodes){
            for (auto nd: nodes){
                add_to_det(nd);
            }
        }

        void add_to_det(std::vector<NodeWrap*>& nws){
            for (auto nw: nws){
                add_to_det(nw);
            }
        }

        void add_to_det(int cycle){
            sampling_vec.push_back(cycle);
        }

        void add_to_det(Node* nd){
            assert(nd != nullptr);
            sampling_vec.push_back(nd->get_cycle_used());
        }
        void add_to_det(NodeWrap* nw){
            assert(nw != nullptr);
            sampling_vec.push_back(nw->get_cycle_used());
        }

        int get_max_cycle_horizon(){
            assert(!sampling_vec.empty());
            int test_val = sampling_vec[0];

            for (auto cycle: sampling_vec){
                /**case detect in consistent in sub block*/
                if (cycle == IN_CONSIST_CYCLE_USED){
                    return IN_CONSIST_CYCLE_USED;
                }
                /** check that it is equal to other*/

                test_val = std::max(test_val, cycle);
            }
            assert(test_val >= 0);
            /** return only when sampling is all equal*/
            return test_val;
        }

        int get_same_cycle_horizon(){
            assert(!sampling_vec.empty());
            int test_val = sampling_vec[0];
            for (auto cycle: sampling_vec){
                if (cycle == IN_CONSIST_CYCLE_USED){
                    return IN_CONSIST_CYCLE_USED;
                }
                if (test_val != cycle){
                    return IN_CONSIST_CYCLE_USED;
                }
            }
            return test_val;
        }

        int get_cycle_vertical(){
            assert(!sampling_vec.empty());
            int cycle_used = 0;
            for (auto sub_cycle: sampling_vec){
                if (sub_cycle == IN_CONSIST_CYCLE_USED){
                    return IN_CONSIST_CYCLE_USED;
                }
                cycle_used += sub_cycle;

            }
            return cycle_used;
        }

        /** search for node that have match to input cycle skip if node is nullptr*/
        static Node*get_match_node_ptr(const std::vector<Node*>& nds, int cycle){
            assert(cycle != IN_CONSIST_CYCLE_USED);
            ////// we do not allow in consist cycle to be matched with node
            for (auto nd : nds){
                if (nd == nullptr){
                    continue;
                }
                if (nd->get_cycle_used() == cycle){
                    return nd;
                }
            }
            return nullptr;
        }

        /** search for node wrap that have match to input cycle skip if node is nullptr*/
        static NodeWrap*get_match_node_wrap_ptr(const std::vector<NodeWrap*>& nws, int cycle){
            assert(cycle != IN_CONSIST_CYCLE_USED);
            for (auto nw: nws){
                if (nw == nullptr){
                    continue;
                }
                if (cycle == nw->get_cycle_used()){
                    return nw;
                }
            }
            return nullptr;
        }

    };

    /** get the exit node of matched node that got same cycle used*/
    static Node*get_match_node_from_nds_or_nws_ptr(const std::vector<Node*>& nds,
                                          const std::vector<NodeWrap*>& nws,
                                          int cycle
    ){

        assert(cycle >= 0);

        Node* matched_node = NodeWrapCycleDet::get_match_node_ptr(nds, cycle);

        if (matched_node != nullptr){
            return matched_node;
        }
        NodeWrap* matched_node_wrap = NodeWrapCycleDet::get_match_node_wrap_ptr(nws, cycle);

        if (matched_node_wrap != nullptr){
            return matched_node_wrap->get_exit_node();
        }

        return nullptr;

    }

    /** get the any exit node that first*/
    static Node*get_any_node_from_nds_or_nws_ptr(const std::vector<Node*>& nds,
                                        const std::vector<NodeWrap*>& nws){
        for (auto nd : nds){
            if (nd != nullptr){
                return nd;
            }
        }
        for (auto nw : nws){
            if (nw != nullptr){
                return nw->get_exit_node();
            }
        }
        return nullptr;
    }

}

#endif //KATHRYN_NODEWRAP_H
