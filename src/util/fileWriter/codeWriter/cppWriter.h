//
// Created by tanawin on 19/7/2024.
//

#ifndef CPPWRITER_H
#define CPPWRITER_H
#include "codeBaseWriter.h"
#include <cassert>
#include <utility>


namespace kathryn{

    constexpr int CXX_IDENT = 4;


struct CbIfCxx;
struct CbSwitchCxx;
struct CbFuncDec;

constexpr char CXX_ULL_SUFFIX [] = "ULL";

struct CbBaseCxx: CbBase{

    CbBaseCxx(): CbBase(){}
    ~CbBaseCxx()  = default;
    virtual CbIfCxx&     addIf(std::string condition);
    virtual CbSwitchCxx& addSwitch(std::string switchIdent);
    virtual CbBaseCxx&   addSubBlock();
    std::string          toString(int ident) override;


};


struct CbIfCxx: CbBaseCxx{

    bool               _markAsSubChain = false; //// for elif else
    std::string        _cond;
    std::vector<CbIfCxx*> _contBlock;

    CbIfCxx(bool isSubChain, std::string condtion);
    ~CbIfCxx(){
        for(CbIfCxx* contBlock: _contBlock){
            delete contBlock;
        }
    }

    CbIfCxx& addElif(std::string condition);
    std::string toString(int ident) override;

};

struct CbSwitchCxx: CbBaseCxx{
    std::string _switchIdent;
    bool isDefaultOccure = false;
    std::vector<int> _caseIdents;

    CbSwitchCxx(std::string switchIdent):
            _switchIdent(std::move(switchIdent)){}

    /////// disable the unused function
    CbIfCxx  & addIf(std::string condition) override{assert(false);}
    CbBaseCxx& addSubBlock() override{assert(false);}

    CbBaseCxx&  addCase(int caseVal);
    std::string toString(int ident) override;

};

}

#endif //CPPWRITER_H
