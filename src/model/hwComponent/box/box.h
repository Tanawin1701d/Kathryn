//
// Created by tanawin on 10/4/2567.
//

#ifndef KATHRYN_BOX_H
#define KATHRYN_BOX_H


#include "model/hwComponent/expression/nest.h"

namespace kathryn{

    struct Box:  public Identifiable,
                public HwCompControllerItf,
                public ModelDebuggable
                {
    protected:
        /**collect meta data for hw component (reg, wire, val expression)    memblk is not included*/
        std::vector<NestMeta> _nestMetas;
        std::vector<Box*>     _recurBoxs;
        /** init communication to controller*/
        void com_init() override;
        /** get meta data*/
        std::vector<NestMeta>& getNestMetas();
        std::vector<Box*>&     getSubBox();
        /** collectAssignMeta to be a asm node */
        void collectAssignMeta(Box& rhsBox,
                                bool isBlockAsm,
                                std::vector<AssignMeta*>& resultCollector);

        void buildAsmNode(Box& rhsBox, bool isBlockAsm);


    public:
        int i = 0;
        explicit Box();
        /**add meta data from provided from controller*/
        void addNestMeta(NestMeta nestMeta);
        void addSubBox(Box* subBox);
        /**com final*/
        void com_final() override;

        Box& operator = (Box& rhs);
        Box& operator <<= (Box& rhs);

        /** for debug*/
        std::string getMdIdentVal() override;
        void addMdLog(MdLogVal* mdLogVal) override;

    };

}

#endif //KATHRYN_BOX_H
