//
// Created by tanawin on 24/6/2024.
//

#ifndef WIREIO_H
#define WIREIO_H

#include"wire.h"
#include"wireSubType.h"

namespace kathryn{


    /**
     *
     * for now wireio is not exposed to user it use as internal generating route
     *
     */
    class WireIo: public Wire{

    protected:

        WIRE_IO_TYPE _wireIoType;

    public:
        WireIo(int size, WIRE_IO_TYPE wireIoType);

        void buildHierarchy(Module* masterModule);

        void connectTo(Operable* opr, bool directAdded){
            ////// directAdded is add to translated event which no require routing any more
            ///// if it is false add to default updateEvent to normal vec it must be routing first
            assert(opr != nullptr);
            assert(opr->getOperableSlice().getSize() == getOperableSlice().getSize());
            assert(_genEngine != nullptr);
            //////////////////////// directly to system
            if (directAdded){
                _genEngine->addDirectUpdateEvent(
                    new UpdateEvent({
                        nullptr,
                        nullptr,
                        opr,
                        getOperableSlice(),
                        DEFAULT_UE_PRI_MIN
                        }));
            }else{
                addUpdateMeta(
                    new UpdateEvent({
                    nullptr,
                    nullptr,
                    opr,
                    getOperableSlice(),
                    DEFAULT_UE_PRI_MIN


                    }));
            }
        }

        [[nodiscard]]
        WIRE_IO_TYPE getWireIoType()const{return _wireIoType;}

        void createLogicGen() override;
    };

}

#endif //WIREIO_H
