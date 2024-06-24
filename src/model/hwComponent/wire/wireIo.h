//
// Created by tanawin on 24/6/2024.
//

#ifndef WIREIO_H
#define WIREIO_H

#include"wire.h"

namespace kathryn{

    enum WIRE_IO_TYPE{
        WIRE_IO_INPUT,
        WIRE_IO_OUTPUT,
        WIRE_IO_INTER
    };

    class WireIo: public Wire{

    protected:

        WIRE_IO_TYPE _wireIoType;

    public:
        WireIo(int size, WIRE_IO_TYPE wireIoType);

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

        [[nodiscard]]
        WIRE_IO_TYPE getWireIoType()const{return _wireIoType;}
    };

}

#endif //WIREIO_H
