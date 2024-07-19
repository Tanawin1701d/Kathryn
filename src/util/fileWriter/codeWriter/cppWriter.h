//
// Created by tanawin on 19/7/2024.
//

#ifndef CPPWRITER_H
#define CPPWRITER_H
#include "codeBaseWriter.h"
#include <cassert>


namespace kathryn{

    constexpr int CXX_IDENT = 4;


struct CbIfCxx;
struct CbFuncDec;

struct CbBaseCxx: CbBase{

    ~CbBaseCxx() override = default;
    CbIfCxx& addIf(std::string condition);
    virtual CbFuncDec& addFuc(std::string funcName, std::string msClassName, std::string retType);
    std::string toString(int ident) override;


};


struct CbIfCxx: CbBaseCxx{

    bool               _markAsSubChain = false; //// for elif else
    std::string        _cond;
    std::vector<CbIfCxx> _contBlock;

    CbIfCxx(bool isSubChain, std::string condtion);
    ~CbIfCxx() override = default;

    CbIfCxx& addElif(std::string condition);
    std::string toString(int ident) override;


};

struct CbFuncDec: CbBaseCxx{

    CbFuncDec(std::string funcName, std::string msClassName, std::string retType);
    ~CbFuncDec() override = default;
    CbFuncDec& addFuc(std::string funcName, std::string msClassName, std::string retType) override;
};

}

#endif //CPPWRITER_H
