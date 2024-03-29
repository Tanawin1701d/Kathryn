//
// Created by tanawin on 29/3/2567.
//

#ifndef KATHRYN_IDENTBASE_H
#define KATHRYN_IDENTBASE_H

#include "sim/logicRep/valRep.h"

namespace kathryn{

    extern ull GLOBAL_MODEL_ID;

    ull getLastIdentId();

    class IdentBase{

    protected:
        bool                     _isFinalize = false;
        ull                      _globalId = -1; ///// Id that shared with all others model element
        std::string              _globalName; ////// global name that shared with all other model element
        std::vector<std::string> _inheritName;////// name that inherit from master

    public:
        explicit IdentBase();

        bool getIdentIsFinalize() const;
        void setIdentIsFinalize();

        ull getGlobalId()const {return _globalId;}

        const std::string& getGlobalName() const;
        void setGlobalName(const std::string &globalName);

        const std::vector<std::string>& getInheritName() const;
        void setInheritName(const std::vector<std::string> &inheritName);
        virtual void buildInheritName() = 0;

        std::string concat_inheritName();

        IdentBase& operator = (const IdentBase& rhs);


    };

}

#endif //KATHRYN_IDENTBASE_H
