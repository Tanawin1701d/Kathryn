//
// Created by tanawin on 19/7/2024.
//

#include "cppWriter.h"

#include <utility>

#include "util/str/strUtil.h"

namespace kathryn{
    /**
     *  CbBaseCxx
     */
    CbIfCxx& CbBaseCxx::addIf(std::string condition){
        auto* ifBlock = new CbIfCxx(false,std::move(condition));
        appendSubBlock(ifBlock);
        return *ifBlock;
    }

    CbBaseCxx& CbBaseCxx::addSubBlock(){
        auto* subBlock = new CbBaseCxx();
        appendSubBlock(subBlock);
        return *subBlock;
    }


    std::string CbBaseCxx::toString(int ident){

        //////// the local idx of eachtype
        int nextSbCheckIdx = 0;
        int nextStCheckIdx = 0;

        std::string preRet;
        std::string identVal = genConString(' ', ident);

        for (int mainOrder = 0; mainOrder < lastOrder; mainOrder++){

            if (nextStCheckIdx < _codeSt.size() &&
                _codeSt[nextStCheckIdx].order == mainOrder){
                preRet += identVal;
                if (_codeSt[nextStCheckIdx].isComment){preRet += "////";}
                preRet += identVal + _codeSt[nextStCheckIdx].st + ";";
                if (_codeSt[nextStCheckIdx].ln){preRet += "\n";}
                nextStCheckIdx++;
                continue;
            }

            if (nextSbCheckIdx < _sbOrder.size() &&
                _sbOrder[nextSbCheckIdx] == mainOrder){
                preRet += _subBlocks[nextSbCheckIdx]->toString(ident + CXX_IDENT);
                nextSbCheckIdx++;
                continue;
            }
            assert(false);
        }
        return preRet;
    }

    /**
     *
     * CbIfCxx
     *
     */

    CbIfCxx::CbIfCxx(bool isSubChain, std::string condtion):
    _markAsSubChain(isSubChain),
    _cond(std::move(condtion)){}

    CbIfCxx& CbIfCxx::addElif(std::string condition){
        _contBlock.emplace_back(true,std::move(condition));
        return _contBlock.back();
    }

    std::string CbIfCxx::toString(int ident){
        std::string preRet;
        std::string indentVal = genConString(' ', ident);

        if (!_markAsSubChain){
            preRet += indentVal + "if(" + _cond + ")";
        }else if (!_cond.empty()){
            preRet += "else if(" + _cond + ")";
        }else{
            preRet += "else";
        }

        preRet += "{\n";

        preRet += CbBaseCxx::toString(ident + CXX_IDENT);

        preRet += indentVal + "}";

        for(CbIfCxx& contBlock: _contBlock){
            preRet += contBlock.toString(ident);
        }
        if (!_markAsSubChain){
            preRet += "\n";
        }
        return preRet;
    }

    /**
     *
     * CbFuncDec
     *
     */











}
