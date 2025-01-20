//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_UOP_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_UOP_BASE_H

#include<vector>
#include<string>
#include"opr_base.h"

#include "carolyne/util/checker/checker.h"


    namespace kathryn::carolyne{

        constexpr char OPR_FD_FOP_IDENT_fop[] = "fop";
        constexpr char UOP_UNNAMED[]          = "unname_Uop";

        struct ExecUTM;
        struct UopTypeBase: GenRowMetaable, VizCsvGenTable{
            std::string               _uopName = UOP_UNNAMED;
            std::vector<OprTypeBase*> _srcOprTypes;
            std::vector<OprTypeBase*> _desOprTypes;
            int                       _fopIdentWidth    = -1; ///// the bit size that uop used to ident its functional
            ExecUTM*                  _execUnitTypeMeta = nullptr; //// which type of exec that this uop love to go
            /**
             * todo next check execute engine
             ***/

            ~UopTypeBase() override{
                for(auto oprType : _srcOprTypes){delete oprType;}
                for(auto oprType:  _desOprTypes){delete oprType;}
            }

            void setFopIdentWidth(int sz){
                crlAss(sz > 0, "fop bit with in uop: " + _uopName + " must have size > 0");
                _fopIdentWidth = sz;
            }

            void setExecUnitType(ExecUTM* execUnitType){
                crlAss(execUnitType != nullptr, "set exec unit to exec unit type should not be nullptr");
                _execUnitTypeMeta = execUnitType;
            }


            /***
             *
             * internal management
             *
             */

            void addOprType(OprTypeBase* rhs, bool isSrc){ //// else or will be destination
                crlAss(rhs != nullptr, "add opr to uop but opr is null");
                auto oprTypeRepo = isSrc ? _srcOprTypes: _desOprTypes;
                oprTypeRepo.push_back(rhs);
            }


            std::vector<OprTypeBase*> getOprTypes(bool isSrc){
                return isSrc ? _srcOprTypes: _desOprTypes;
            }

            /**
             * equal identification
             *
             */

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

            /**
             *  gen row meta data
             */

            RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) override{
                //////// this generate only the field for uop (exclude the operand)
                //////// for now we accept all
                crlAss(_fopIdentWidth > 0, "fop bit with in uop: " + _uopName + " must have size > 0 while row generating");
                RowMeta row;
                row.addField(OPR_FD_FOP_IDENT_fop, _fopIdentWidth);
                return row;
            }

            ////// pool it with operand and send them back with pooled rowmeta type
            CsvTable genTable() override{
                RowMeta resultRow(genRowMeta(CGM_DECODE, 0));
                for (OprTypeBase* srcOprType: _srcOprTypes){
                    RowMeta oprRow = srcOprType->genRowMeta(CGM_DECODE, 0);
                    resultRow += oprRow;
                }
                for (OprTypeBase* desOprType: _desOprTypes){
                    RowMeta oprRow = desOprType->genRowMeta(CGM_DECODE, 0);
                    resultRow += oprRow;
                }

                CsvTable uopTable(resultRow.genTable());
                uopTable.setTableName(_uopName);
                return uopTable;
            }

        };

        /**
         * UopMatcherBase
         *  aim to match the raw instuction in each opr
         * **/
        struct UopMatcherBase: SliceMatcher{
            int _instrWidth = -1;
            std::vector<OprMatcherBase*> _srcOprMatcher;
            std::vector<OprMatcherBase*> _desOprMatcher;
            UopTypeBase* _uopType = nullptr;

            UopMatcherBase(
               int                        instrWidth,
               UopTypeBase*               uopType):
            SliceMatcher(instrWidth),
            _instrWidth(instrWidth),
            _uopType(uopType){
                crlAss(_instrWidth > 0, "uopMatcher got instruction width < 0");
                crlAss(_uopType != nullptr, "uopType of the matcher must not be nullptr");
            }

            virtual ~UopMatcherBase(){
                for (OprMatcherBase* oprMatcherBase: _srcOprMatcher){ delete oprMatcherBase;}
                for (OprMatcherBase* oprMatcherBase: _desOprMatcher){ delete oprMatcherBase;}
            }

            void addOprMatcher(OprMatcherBase* oprMatcher, bool isSrc){
                crlAss(oprMatcher != nullptr, "add opr matcher to uop cannot be null");
                if (isSrc){
                    _srcOprMatcher.push_back(oprMatcher);
                }else{
                    _desOprMatcher.push_back(oprMatcher);
                }
            }

            void setOprMatcher(const std::vector<OprMatcherBase*>& oprMatchers, bool isSrc){
                for (OprMatcherBase* oprMatcher : oprMatchers){
                    addOprMatcher(oprMatcher, isSrc);
                }
            }

            std::vector<OprMatcherBase*> getOprMatcher(bool isSrc){
                return isSrc ? _srcOprMatcher : _desOprMatcher;
            }
        };


    }


#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_BACKEND_UOP_UOP_BASE_H
