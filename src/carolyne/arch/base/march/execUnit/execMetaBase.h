//
// Created by tanawin on 31/12/2024.
//

#ifndef src_carolyne_arch_base_march_execUnit_EXECMETABASE_H
#define src_carolyne_arch_base_march_execUnit_EXECMETABASE_H
#include <vector>
#include "carolyne/arch/base/isa/uop/opr_base.h"
#include "carolyne/arch/base/util/rowMeta.h"

namespace kathryn{
    namespace carolyne{

        /** Unit type meta*/
        struct ExecUTM{
            std::string _execType;

            virtual ~ExecUTM() = default;
            virtual void createOp(){
                crlAss(false, "i don't know what i have done.");
            }

        };

        struct ExecRsvUnitMatch{
            ExecUTM* _execType = nullptr;
            int      _amt      = -1;

            ExecRsvUnitMatch(ExecUTM* execType,int amt):
            _execType(execType),
            _amt(amt){
                crlAss(_execType != nullptr, "execMatch cannot be null");
                crlAss(_amt > 0, "amt unit must > 0");
            }

            [[nodiscard]]
            int      getAmtUnit() const {return _amt;}
            [[nodiscard]]
            ExecUTM* getUTM()     const {return _execType;}
        };

    }
}

#endif //src_carolyne_arch_base_march_execUnit_EXECMETABASE_H
