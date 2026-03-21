//
// Created by tanawin on 24/6/2024.
//

#ifndef WIREAUTO_H
#define WIREAUTO_H

#include "wire.h"
#include "wire_sub_type.h"

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
        WireAuto(int size, WIRE_AUTO_GEN_TYPE wire_io_type);

        void build_hierarchy(Module* master_module);

        void connect_to(Operable* opr1, bool direct_added){
            ////// direct_added is add to translated event which no require routing any more
            ///// if it is false add to default update_event to normal vec it must be routing first
            assert(opr1 != nullptr);
            assert(opr1->get_operable_slice().get_size() == get_operable_slice().get_size());
            assert(_genEngine != nullptr);
            //////////////////////// directly to system
            if (direct_added){
                _genEngine->add_direct_update_event(
                    create_ue_helper(opr1,
                                   get_operable_slice(),
                                   DEFAULT_UE_PRI_MIN,
                                   CM_CLK_FREE,
                                   false)
                );
            }else{
                add_update_meta(
                    create_ue_helper(opr1,
                        get_operable_slice(),
                        DEFAULT_UE_PRI_MIN,
                        CM_CLK_FREE,
                        false)
                );
            }
        }

        [[nodiscard]]
        WIRE_AUTO_GEN_TYPE get_wire_io_type()const{return _wireIoType;}

        void create_logic_gen() override;
    };

}

#endif //WIREIO_H
