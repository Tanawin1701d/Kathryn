//
// Created by tanawin on 7/2/26.
//

#ifndef MODEL_FLOWBLOCK_PIPESTREAM_STREAMELE_H
#define MODEL_FLOWBLOCK_PIPESTREAM_STREAMELE_H

#include "model/flowBlock/seq/seq.h"

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


        StreamEle(SequenceEle* masterEle);
        virtual ~StreamEle();


        void genNode          (CLOCK_MODE cm);
        void setIdentStateId  (ull masterIdx, int subIdx) const;
        void setIntReset      (OprNode* intResetNode    );
        void setHoldNode      (OprNode* holdNode        );
        void addSyncDependency(StreamEle* prevStreamEle,
                               StreamEle* nextStreamEle) const;
        void assignIntStart   (OprNode* intStartNode);
        void addToSystemNodes (std::vector<Node*>& sysNode);

        bool checkAllNodeGen  ();

        void tryAssignIntSig (StateNode* nd) const;
        void tryAssignHoldSig(StateNode* nd) const;

        Node*getEntranceNodePtr()const;



    };

}

#endif //MODEL_FLOWBLOCK_PIPESTREAM_STREAMELE_H