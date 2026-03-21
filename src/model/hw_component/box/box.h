//
// Created by tanawin on 10/4/2567.
//

#ifndef KATHRYN_BOX_H
#define KATHRYN_BOX_H


#include "model/hw_component/expression/nest.h"

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
        std::vector<NestMeta>& get_nest_metas();
        std::vector<Box*>&     get_sub_box();
        /** collect_assign_meta to be a asm node */
        void collect_assign_meta(Box& rhs_box,
                                bool is_block_asm,
                                std::vector<AssignMeta*>& result_collector);

        void build_asm_node(Box& rhs_box, bool is_block_asm);


    public:
        int i = 0;
        explicit Box();
        /**add meta data from provided from controller*/
        void add_nest_meta(NestMeta nest_meta);
        void add_sub_box(Box* sub_box);
        /**com final*/
        void com_final() override;

        Box& operator = (Box& rhs);
        Box& operator <<= (Box& rhs);

        /** for debug*/
        std::string get_md_ident_val() override;
        void add_md_log(MdLogVal* md_log_val) override;

    };

}

#endif //KATHRYN_BOX_H
