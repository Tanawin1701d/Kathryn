//
// Created by tanawin on 29/3/2567.
//

#ifndef KATHRYN_IDENTBASE_H
#define KATHRYN_IDENTBASE_H

#include "sim/logicRep/valRep.h"

namespace kathryn{

    extern ull GLOBAL_MODEL_ID;

    ull get_last_ident_id();

    class IdentBase{

    protected:
        bool                     _is_finalized = false; ///// is the variable below stable
        ull                      _global_id    = -1;    ///// Id that shared with all others model element
        std::string              _global_name;          ////// global name that shared with all other model element
        std::vector<std::string> _inherit_name;         ////// name that inherit from master

    public:
        explicit IdentBase();

        bool is_ident_finalized() const;
        void finalize_ident    ();

        ull                get_global_id     ()const {return _global_id;}
        const std::string& get_global_name() const;
        void               set_global_name   (const std::string &global_name);

        const std::vector<std::string>&
                           get_inherit_name() const;
        void               set_inherit_name(const std::vector<std::string> &inherit_name);
        std::string        concat_inheritName();


        virtual void       build_inherit_name() = 0;



        IdentBase& operator = (const IdentBase& rhs);


    };

}

#endif //KATHRYN_IDENTBASE_H
