//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_UOP_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_UOP_BASE_H

#include<vector>
#include<string>
#include"opr_base.h"
#include "carolyne/util/checker/checker.h"

namespace kathryn{
    namespace carolyne{

        /**
         * UopMatcherBase
         *  aim to match the raw instuction in each opr
         * **/
        struct UopTypeBase;
        struct UopMatcherBase{
            int _instrWidth = -1;
            std::vector<OprMatcherBase*> _srcOprMatcher;
            std::vector<OprMatcherBase*> _desOprMatcher;
            std::vector<Slice>           _matchedSlices; //// the slice in raw fetch instruction that match with these
            UopTypeBase* _uopType = nullptr;

            UopMatcherBase(
               int                 instrWidth,
               const std::vector<Slice>&  matchedSlices,
               UopTypeBase*        uopType):
            _instrWidth(instrWidth),
            _matchedSlices(matchedSlices),
            _uopType(uopType){
                crlAss(_instrWidth > 0, "uopMatcher got instruction width < 0");
                crlAss(_uopType != nullptr, "uopType of the matcher must not be nullptr");

                for (Slice sl: _matchedSlices){
                    crlAss(sl.checkValidSlice(),
                           "invalid uop match slice " + std::to_string(sl.start) +
                           " " + std::to_string(sl.stop));
                    crlAss(sl.stop < _instrWidth,
                           "uop matching sl.stop = " + std::to_string(sl.stop) +
                           "exceed instrwidth " + std::to_string(_instrWidth));
                }
            }
        };



        struct UopTypeBase{
            std::string _uopName;
            std::vector<OprTypeBase*> _srcOprTypes;
            std::vector<OprTypeBase*> _desOprTypes;
            /**
             * todo next check execute engine
             ***/

            virtual ~UopTypeBase(){
                for(auto oprType : _srcOprTypes){delete oprType;}
                for(auto oprType:  _desOprTypes){delete oprType;}
            }

            void addOprType(OprTypeBase* rhs, bool isSrc){ //// else or will be destination
                crlAss(rhs != nullptr, "add opr to uop but opr is null");
                auto oprTypeRepo = isSrc ? _srcOprTypes: _desOprTypes;
                oprTypeRepo.push_back(rhs);
            }

            std::vector<OprTypeBase*> getOprTypes(bool isSrc){
                return isSrc ? _srcOprTypes: _desOprTypes;
            }

            bool checkOprs(const std::vector<OprTypeBase*>& leftOprTypes,
                           const std::vector<OprTypeBase*>& rightOprTypes
                           ){

                if (leftOprTypes.size() != rightOprTypes.size()){
                    return false;
                }

                for (int idx = 0; idx < leftOprTypes.size(); idx++){
                    bool checkResult =
                    leftOprTypes[idx]->isEqualType(*rightOprTypes[idx]);

                    if(!checkResult){
                        return false;
                    }
                }
                ///// all operand is equal
                return true;
            }

            virtual bool isEqualTypeDeep(const UopTypeBase& rhs) = 0;

            bool isEqualType(const UopTypeBase& rhs){

                if (_uopName != rhs._uopName){return false;}
                ///// check src opr
                bool checkSrcOprTypes = checkOprs(_srcOprTypes, rhs._srcOprTypes);
                if (!checkSrcOprTypes){return false;}

                ///// check des opr type
                bool checkDesOprTypes = checkOprs(_desOprTypes, rhs._desOprTypes);
                if (!checkDesOprTypes){return false;}

                ///// deep check
                return isEqualTypeDeep(rhs);
            }
        };


    }
}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_UOP_BASE_H
