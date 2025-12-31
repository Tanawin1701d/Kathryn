//
// Created by tanawin on 24/6/2024.
//

#ifndef WIREAUTO_H
#define WIREAUTO_H

#include"wire.h"
#include"wireSubType.h"

namespace kathryn{


    /**
     *
     * for now wireio is not exposed to user it use as internal generating route
     *
     */
    class WireAuto: public Wire{

    protected:

        WIRE_AUTO_GEN_TYPE _wireIoType;

    public:
        WireAuto(int size, WIRE_AUTO_GEN_TYPE wireIoType);

        void buildHierarchy(Module* masterModule);

        void connectTo(Operable* opr1, bool directAdded){
            ////// directAdded is add to translated event which no require routing any more
            ///// if it is false add to default updateEvent to normal vec it must be routing first
            assert(opr1 != nullptr);
            assert(opr1->getOperableSlice().getSize() == getOperableSlice().getSize());
            assert(_genEngine != nullptr);
            //////////////////////// directly to system
            if (directAdded){
                _genEngine->addDirectUpdateEvent(
                    createUEHelper(opr1,
                                   getOperableSlice(),
                                   DEFAULT_UE_PRI_MIN,
                                   CM_CLK_FREE,
                                   false)
                );
            }else{
                addUpdateMeta(
                    createUEHelper(opr1,
                        getOperableSlice(),
                        DEFAULT_UE_PRI_MIN,
                        CM_CLK_FREE,
                        false)
                );
            }
        }

        [[nodiscard]]
        WIRE_AUTO_GEN_TYPE getWireIoType()const{return _wireIoType;}

        void createLogicGen() override;
    };

}

#endif //WIREIO_H
