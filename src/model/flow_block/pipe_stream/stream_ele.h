//
// Created by tanawin on 7/2/26.
//

#ifndef MODEL_FLOWBLOCK_PIPESTREAM_STREAMELE_H
#define MODEL_FLOWBLOCK_PIPESTREAM_STREAMELE_H

#include "model/flow_block/seq/seq.h"

namespace kathryn{

    class StreamEle{
    public:


        //////////// at head of pipeline block
        StateNode*   _waitPrevNode  = nullptr;
        PseudoNode*  _entNode       = nullptr;
        PseudoNode*  _acceptForPrev = nullptr;

        //////////// at middle of block
        SequenceEle* _masterElement = nullptr;

        //////////// at bottom of pipline block
        StateNode*   _waitNextNode  = nullptr;
        PseudoNode*  _readyForNext  = nullptr;
        PseudoNode*  _syncedNext    = nullptr;


        StreamEle(SequenceEle* master_ele);
        virtual ~StreamEle();


        void gen_node          (CLOCK_MODE cm);
        void set_ident_state_id  (ull master_idx, int sub_idx) const;
        void set_int_reset      (OprNode* int_reset_node    );
        void set_hold_node      (OprNode* hold_node        );
        void add_sync_dependency(StreamEle* prev_stream_ele,
                               StreamEle* next_stream_ele) const;
        void assign_int_start   (OprNode* int_start_node);
        void add_to_system_nodes (std::vector<Node*>& sys_node);

        bool check_all_node_gen  ();

        void try_assign_int_sig (StateNode* nd) const;
        void try_assign_hold_sig(StateNode* nd) const;

        Node*get_entrance_node_ptr()const;



    };

}

#endif //MODEL_FLOWBLOCK_PIPESTREAM_STREAMELE_H