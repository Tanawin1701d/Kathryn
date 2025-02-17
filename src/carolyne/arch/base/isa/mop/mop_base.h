//
// Created by tanawin on 29/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_MOP_MOP_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_MOP_MOP_BASE_H

#include <utility>

#include"carolyne/arch/base/isa/uop/uop_base.h"

namespace kathryn{
    namespace carolyne{

        struct MopTypeBase: SliceMatcher, VizCsvGenable{
            std::string                  _mopName;
            int                          _mopBitWidth = -1;
            int                          _mopIdentValue;
            std::vector<UopTypeBase*>    _uopTypes;
            std::vector<UopMatcherBase*> _uopMatchers;

            explicit MopTypeBase(std::string mopName, int mopBitWidth, int mopIdentValue):
            SliceMatcher(mopBitWidth),
            _mopName(std::move(mopName)),
            _mopBitWidth(mopBitWidth),
            _mopIdentValue(mopIdentValue){
                crlAss(_mopBitWidth > 0, "mopIdentWidth must greater than 0");
            }

            virtual ~MopTypeBase(){
                for(auto uopMatcher: _uopMatchers){delete uopMatcher;}
            }

            void addUop(UopTypeBase* uopType){
                crlAss(uopType != nullptr, "add uop to mop: uop cannot be nullptr");
                _uopTypes.push_back(uopType);
            }

            void addUopMatcher(UopMatcherBase* uopMatcher){
                crlAss(uopMatcher != nullptr, "add uopMatcher to mopMatcher cannot be null");
                _uopMatchers.push_back(uopMatcher);
            }

            int                          getMopBitWidth()   const {return _mopBitWidth;}
            std::vector<UopTypeBase*>    getUopTypes()      const {return _uopTypes;}
            std::vector<UopMatcherBase*> getUopMatcher()    const {return _uopMatchers;}
            int                          getMopIdentValue() const {return _mopIdentValue;}

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

            /** matcher management*/
            virtual void generateMatchers() = 0;

            /**gen visualization*/
            void visual(CsvGenFile& genFile) override{

                genFile.addRowData(_mopName + "sz<" + std::to_string(_mopBitWidth) + ">");
                // for (UopTypeBase* uopType: _uopTypes){
                //     genFile.addData(uopType->genTable());
                // }
            }

        };

    }
}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_MOP_MOP_BASE_H
