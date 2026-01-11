//
// Created by tanawin on 27/11/25.
//

#include "verilogWriter.h"

#include <utility>
#include "util/str/strUtil.h"

namespace kathryn{
    /**
     *  CbBaseVerilog
     */
    CbIfVerilog& CbBaseVerilog::addIf(std::string condition){
        auto* ifBlock = new CbIfVerilog(false,std::move(condition));
        appendSubBlock(ifBlock);
        return *ifBlock;
    }

    CbBaseVerilog& CbBaseVerilog::addSubBlock(){
        auto* subBlock = new CbBaseVerilog();
        appendSubBlock(subBlock);
        return *subBlock;
    }

    CbAlwaysVerilog& CbBaseVerilog::addAlways(Verilog_SEN_TYPE senType, std::string senName){
        auto* alwaysBlock = new CbAlwaysVerilog(senType,std::move(senName));
        appendSubBlock(alwaysBlock);
        return *alwaysBlock;
    }

    CbSwitchVerilog& CbBaseVerilog::addSwitch(std::string switchIdent){
        auto* switchBlock = new CbSwitchVerilog(std::move(switchIdent));
        appendSubBlock(switchBlock);
        return *switchBlock;
    }


    std::string CbBaseVerilog::toString(int ident){

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
                preRet += _subBlocks[nextSbCheckIdx]->toString(ident + Verilog_IDENT);
                nextSbCheckIdx++;
                continue;
            }
            assert(false);
        }
        return preRet;
    }

    /**
     *
     * CbIfVerilog
     *
     */

    CbIfVerilog::CbIfVerilog(bool isSubChain, std::string condtion):
    _markAsSubChain(isSubChain),
    _cond(std::move(condtion)){}

    CbIfVerilog& CbIfVerilog::addElif(std::string condition){
        _contBlock.emplace_back(true,std::move(condition));
        return _contBlock.back();
    }

    std::string CbIfVerilog::toString(int ident){
        std::string preRet;
        std::string indentVal = genConString(' ', ident);

        if (!_markAsSubChain){
            preRet += indentVal + "if(" + _cond + ")";
        }else if (!_cond.empty()){
            preRet += "else if(" + _cond + ")";
        }else{
            preRet += "else";
        }

        preRet += "begin\n";

        preRet += CbBaseVerilog::toString(ident + Verilog_IDENT);

        preRet += indentVal + "end ";

        for(CbIfVerilog& contBlock: _contBlock){
            preRet += contBlock.toString(ident);
        }
        if (!_markAsSubChain){
            preRet += "\n";
        }
        return preRet;
    }

    /**
     *
     * CbAlwaysVerilog
     *
     */

    CbAlwaysVerilog::CbAlwaysVerilog(Verilog_SEN_TYPE senType, std::string  senName):
    _senType(senType),
    _senName(std::move(senName)){}

    std::string CbAlwaysVerilog::toString(int ident){

        assert(_senType >= 0 && _senType < VLST_CNT);

        std::string preRet;
        std::string indentVal = genConString(' ', ident);

        std::string senName = (_senType == VLST_ALWAYS) ? "*" : (Verilog_SEN_TYPE_STR[_senType] + " " +_senName);

        preRet += indentVal + "always @(" + senName + " ) begin\n";
        preRet += CbBaseVerilog::toString(ident + Verilog_IDENT);
        preRet += indentVal + "end";

        return preRet;

    }

    /**
     *
     * CbSwitchVerilog
     *
     */

    CbBaseVerilog& CbSwitchVerilog::addCase(int caseVal){
        if (caseVal == -1){
            assert(!isDefaultOccure);
            isDefaultOccure = true;
        }
        _caseIdents.push_back(caseVal);
        return CbBaseVerilog::addSubBlock();
    }

    std::string CbSwitchVerilog::toString(int ident){
        std::string preRet;
        std::string indentVal = genConString(' ', ident);
        std::string indentValInside = genConString(' ', ident + Verilog_IDENT);

        preRet += indentVal + "case(" + _switchIdent + ")\n";

        assert(_subBlocks.size() == _caseIdents.size());
        for (int idx = 0; idx < _subBlocks.size(); idx++){

            std::string finalizedCaseId = _caseIdents[idx] == -1 ? "default"
                                                                 : std::to_string(_caseIdents[idx]);
            preRet += indentValInside + finalizedCaseId + ":\n";
            preRet += indentValInside + " begin\n";
            preRet += _subBlocks[idx]->toString(ident + 2* Verilog_IDENT) + "\n";
            preRet += indentValInside + " end\n";
        }

        preRet += indentVal + "endcase\n";
        return preRet;
    }



}
