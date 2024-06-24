//
// Created by tanawin on 24/6/2024.
//

#ifndef WIREIO_H
#define WIREIO_H

#include"wire.h"

namespace kathryn{

    class WireIo: public Wire{

        bool _isInput;

    public:
        WireIo(int size, bool isInput);

        void buildHierarchy(Module* masterModule);

        void connectTo(Operable* opr){
            assert(opr != nullptr);
            assert(opr->getOperableSlice().getSize() == getOperableSlice().getSize());
            std::vector<UpdateEvent*>& updateVecs = getUpdateMeta();
            assert(updateVecs.empty());
            updateVecs.push_back(
                new UpdateEvent({
                    nullptr,
                    nullptr,
                    opr,
                    getOperableSlice(),
                    DEFAULT_UE_PRI_MIN
                    })
            );
        }
    };

}

#endif //WIREIO_H
