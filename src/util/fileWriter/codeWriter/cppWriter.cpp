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

    CbSwitchCxx& CbBaseCxx::addSwitch(std::string switchIdent){
        auto* switchBlock = new CbSwitchCxx(std::move(switchIdent));
        appendSubBlock(switchBlock);
        return *switchBlock;
    }

    CbBaseCxx& CbBaseCxx::addSubBlock(){
        auto* subBlock = new CbBaseCxx();
        appendSubBlock(subBlock);
        return *subBlock;
    }


    std::string CbBaseCxx::toString(int ident){

        //////// the local idx of eachtype

        /////////// sb for subblock index
        /////////// st for statement index
        int nextSbCheckIdx = 0;
        int nextStCheckIdx = 0;

        std::string preRet;
        std::string identVal = genConString(' ', ident);

        for (int mainOrder = 0; mainOrder < lastOrder; mainOrder++){

            ///////// we have to pool it inorder no mater it is subblock or statement

            if (nextStCheckIdx < _codeSt.size() &&
                _codeSt[nextStCheckIdx].order == mainOrder){
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
        CbIfCxx* elifBlock = new CbIfCxx(true,std::move(condition));
        _contBlock.push_back(elifBlock);
        return *elifBlock;
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

        for(CbIfCxx* contBlock: _contBlock){
            assert(contBlock != nullptr);
            preRet += contBlock->toString(ident);
        }
        if (!_markAsSubChain){
            preRet += "\n";
        }
        return preRet;
    }

    /**
     *
     * CbSwitchFunc
     *
     */

    CbBaseCxx& CbSwitchCxx::addCase(int caseVal){

        if (caseVal == -1){
            assert(!isDefaultOccure);
            isDefaultOccure = true;
        }
        _caseIdents.push_back(caseVal);
        return CbBaseCxx::addSubBlock();

    }

    std::string CbSwitchCxx::toString(int ident){
        std::string preRet;
        std::string indentVal = genConString(' ', ident);
        std::string indentValInside = genConString(' ', ident + CXX_IDENT);

        preRet += indentVal + "switch(" + _switchIdent + "){\n";

        assert(_subBlocks.size() == _caseIdents.size());
        for (int idx = 0; idx < _subBlocks.size(); idx++){

            /////// add break to break
            _subBlocks[idx]->addSt("break", true);

            std::string finalizedCaseStr = _caseIdents[idx] == -1 ? "default"
                                                                 : "case " + std::to_string(_caseIdents[idx]);
            preRet += indentValInside +  finalizedCaseStr + ":\n";
            preRet += indentValInside + " {\n";
            preRet += _subBlocks[idx]->toString(ident + 2* CXX_IDENT) + "\n";
            preRet += indentValInside + " }\n";
        }

        preRet += indentVal + "}";
        return preRet;
    }



}
