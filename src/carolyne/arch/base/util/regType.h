//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_UTIL_REGTYPE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_UTIL_REGTYPE_H

#include <cstdint>
#include "carolyne/util/checker/checker.h"

namespace kathryn{
    namespace carolyne{

        constexpr char RTM_FD_idx  [] = "idx";
        constexpr char RTM_FD_value[] = "value";

        //// the struct store architecture register type meta data
        constexpr int MAX_AMT_REG   = 64;
        constexpr int MAX_REG_WIDTH = 64;

        struct RegTypeMeta{
            const int INDEX_SIZE = -1;
            const int AMT_REG    = -1;
            const int REG_WIDTH  = -1;

            ///// hardwireValue
            ///// the index is register id start from 0;
            /// the value is hardware value if the hardwire status is false the value is not gurantee
            uint64_t _hardwireValue [MAX_AMT_REG];
            bool     _hardwireStatus[MAX_AMT_REG]; ///// the index is register id

            explicit RegTypeMeta(int index_size, int reg_width):
            AMT_REG(1 << index_size),
            REG_WIDTH(reg_width),
            INDEX_SIZE(index_size){

                ////// check the input
                crlAss(INDEX_SIZE, "INDEX_SIZE cannot <= 0");
                crlAss(AMT_REG > 0, "AMT_REG cannot <= 0");
                crlAss(AMT_REG <= MAX_AMT_REG, "AMT_REG must <= MAX_AMT_REG");
                crlAss(REG_WIDTH > 0, "REG_WIDTH cannot < 0");
                crlAss(REG_WIDTH <= MAX_AMT_REG, "REG_WIDTH must >= MAX_AMT_REG");
                ////// clean the data
                std::fill(_hardwireStatus,_hardwireStatus + MAX_AMT_REG,false);
            }

            RegTypeMeta(const RegTypeMeta& rhs):
            INDEX_SIZE(rhs.INDEX_SIZE),
            AMT_REG(rhs.AMT_REG),
            REG_WIDTH(rhs.REG_WIDTH)
            {
                std::copy(rhs._hardwireValue, rhs._hardwireValue + MAX_AMT_REG, _hardwireValue);
                std::copy(rhs._hardwireStatus, rhs._hardwireStatus + MAX_AMT_REG, _hardwireStatus);
            }

            /**
             * normal getter
             * */
            int getIndexWidth() const{return INDEX_SIZE;}
            int getAmtReg    () const{return AMT_REG;}
            int getRegWidth  () const{return REG_WIDTH;}

            /**
             * hardwire management
             * */
            void setHardWireReg(int index, uint64_t value){
                crlAss(index < AMT_REG, "setHardware index out of range");
                _hardwireStatus[index] = true;
                _hardwireValue [index] = value;
            }
            bool isHardWireReg(int index){
                crlAss(index < AMT_REG, "isHardwireReg index out of range");
                return _hardwireStatus[index];
            }
            uint64_t getHardWireValue(int index){
                crlAss(index < AMT_REG, "getHardware Value out of range");
                return _hardwireValue[index];
            }


            /***
             * check equal
             */

            bool operator == (const RegTypeMeta& rhs)const{

                bool prelimCheck =
                (INDEX_SIZE     == rhs.INDEX_SIZE) &&
                (AMT_REG        == rhs.AMT_REG) &&
                (REG_WIDTH      == rhs.REG_WIDTH);

                if (!prelimCheck){return false;}

                for (int i = 0; i < AMT_REG; i++){
                    if (_hardwireStatus[i] != rhs._hardwireStatus[i]){
                        return false;
                    }
                    ////// new status for this index is equal
                    if (_hardwireValue[i] != rhs._hardwireValue[i]){
                        return false;
                    }
                }

                return true;

            }

        };

    }
}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_UTIL_REGTYPE_H