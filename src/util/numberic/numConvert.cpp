//
// Created by tanawin on 19/1/2567.
//

#include <algorithm>
#include "numConvert.h"


namespace kathryn{

    int AMT_IDENT = 6;
    int ASCII_START = 48;

    char sizeIdent[6] = {'b', 'B',
                         'h', 'H',
                         'd', 'D'
                         };

    int amtContainPerChar[6] = {2 , 2,
                                16 , 16,
                                10, 10
                      };

    int amtBitUsedPerChar[6]  = {1, 1,
                                 4, 4,
                                 -1, -1
    };


    bool isContainIdent(char ident){
        return std::any_of(sizeIdent,
                           sizeIdent + AMT_IDENT,
                           [&](char x){return x == ident;});

    }

    int idxOfIdent(char ident){
        auto iter =  std::find(sizeIdent, sizeIdent + AMT_IDENT, ident);
        int idx = (int)(iter - sizeIdent);
        assert(idx < AMT_IDENT);
        return idx;
    }

    /** eleSize must align with power of 2 (1 ,2, 4, 8)*/
    void buildValVec(std::string rawVal, ValRep& result, int eleSize){
        int resultBitIdx = 0;
        ull* resultValPtr = result.getVal();
        assert( (rawVal.size() * eleSize) <= result.getLen());
        for (int i = (int)rawVal.size()-1; i >= 0; i--){
            int vecIdx = resultBitIdx / ((int)(sizeof(ull) << 3));
            int numIdx = resultBitIdx % ((int)(sizeof(ull) << 3));
            ull assignNum;
            assignNum = rawVal[i] - ASCII_START;
            assignNum = assignNum << numIdx;
            resultValPtr[vecIdx] |= assignNum;
            resultBitIdx += eleSize;
        }
    }



    ValRep cvStrToValRep(std::string rawStr){

        assert(rawStr.size() >= 2);

        std::string rawSize;
        std::string rawPrefix;
        std::string rawNumber;

        int idx = 0;
        /**trap emplicit size that delclared by user*/
        while (idx < rawStr.size()){
            if (isContainIdent(rawStr[idx])){
                break;
            }
            rawSize += rawStr[idx];
            idx++;
        }
        /***trap header*/
        assert(idx < rawStr.size());
        assert(isContainIdent(rawStr[idx]));
        assert( (rawStr[idx] != 'd') && (rawStr[idx] != 'D') );
        /** TODO in this version it is not support decimal*/
        rawPrefix += rawStr[idx];
        idx++;
        /**trap value*/
        int identIdx = idxOfIdent(rawPrefix[0]);
        while(idx < rawStr.size()){
            assert((rawStr[idx] < (ASCII_START + amtContainPerChar[identIdx])) &&
                   (rawStr[idx] >= ASCII_START)
            );
            rawNumber += rawStr[idx];
            idx++;
        }
        /********************************************************************/
        int allBitCnt = rawSize.empty() ? ((int)rawNumber.size()) * amtBitUsedPerChar[identIdx]:
                                          atoi(rawSize.c_str());
        /*********************************************************************/
        ValRep res(allBitCnt);
        buildValVec(rawNumber, res, amtBitUsedPerChar[identIdx]);
        return res;
    }

}