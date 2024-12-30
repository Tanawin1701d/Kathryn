//
// Created by tanawin on 29/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_MOP_MOP_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_MOP_MOP_BASE_H

#include"carolyne/arch/base/isa/uop/uop_base.h"

namespace kathryn{
    namespace carolyne{

        struct MopTypeBase{
            std::string _mopName;
            std::vector<UopTypeBase*> _uopTypes;

            explicit MopTypeBase(const std::string& mopName):
            _mopName(mopName){}

            virtual ~MopTypeBase(){
                for(auto uopType: _uopTypes){delete uopType;}
            }

            void addUop(UopTypeBase* uopType){
                crlAss(uopType != nullptr, "add uop to mop: uop cannot be nullptr");
                _uopTypes.push_back(uopType);
            }

            bool checkEqualUopTypes(const std::vector<UopTypeBase*>& lhsUopTypes,
                                    const std::vector<UopTypeBase*>& rhsUopTypes){
                if (lhsUopTypes.size() != rhsUopTypes.size()){
                    return false;
                }

                for(int idx = 0; idx < lhsUopTypes.size(); idx++){
                    bool checkValue = lhsUopTypes[idx]->isEqualType(*rhsUopTypes[idx]);
                    if (!checkValue){
                        return false;
                    }
                }
                return true;
            }

            virtual bool isEqualTypeDeep(const MopTypeBase& rhs) = 0;

            bool isEqualType(const MopTypeBase& rhs){
                if (_mopName != rhs._mopName){return false;}
                ////// check uop type
                bool checkUopEqual = checkEqualUopTypes(_uopTypes, rhs._uopTypes);
                if (!checkUopEqual){return false;}
                ////// check deep
                return isEqualTypeDeep(rhs);
            }
        };

        /**
         * MopMatcherBase
         * aim to match the req instruction in each opr
         */
        struct MopMatcherBase: SliceMatcher{
            int _instrWidth = -1;
            ///////// the index of the uopMatcher is related to mopType
            std::vector<UopMatcherBase*> _uopMatchers;
            MopTypeBase* _mopType = nullptr;

            virtual ~MopMatcherBase(){
                for (UopMatcherBase* umb: _uopMatchers){
                    delete umb;
                }
            }

            MopMatcherBase(int instrWidth, MopTypeBase* mopType):
            SliceMatcher(instrWidth),
            _instrWidth(instrWidth),
            _mopType(mopType){
                crlAss(mopType != nullptr, "add mopType to Mop Matcher cannot be null");
            }

            void addUopMatcher(UopMatcherBase* uopMatcher){
                crlAss(uopMatcher != nullptr, "add uopMatcher to mopMatcher cannot be null");
                _uopMatchers.push_back(uopMatcher);
            }


        };
    }
}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_MOP_MOP_BASE_H
