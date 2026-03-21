//
// Created by tanawin on 29/3/2567.
//

#ifndef KATHRYN_IDENTBASE_H
#define KATHRYN_IDENTBASE_H

#include "sim/logic_rep/val_rep.h"

namespace kathryn{

    extern ull GLOBAL_MODEL_ID;

    ull get_last_ident_id();

    class IdentBase{

    protected:
        bool                     _isFinalize = false;
        ull                      _globalId = -1; ///// Id that shared with all others model element
        std::string              _globalName; ////// global name that shared with all other model element
        std::vector<std::string> _inheritName;////// name that inherit from master

    public:
        explicit IdentBase();

        bool get_ident_is_finalize() const;
        void set_ident_is_finalize();

        ull get_global_id()const {return _globalId;}

        const std::string& get_global_name() const;
        void set_global_name(const std::string &global_name);

        const std::vector<std::string>& get_inherit_name() const;
        void set_inherit_name(const std::vector<std::string> &inherit_name);
        virtual void build_inherit_name() = 0;

        std::string concat_inheritName();

        IdentBase& operator = (const IdentBase& rhs);


    };

}

#endif //KATHRYN_IDENTBASE_H
