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

    CbBaseCxx(): CbBase(){}
    ~CbBaseCxx()  = default;
    CbIfCxx& addIf(std::string condition);
    CbBaseCxx& addSubBlock();
    std::string toString(int ident) override;


};


struct CbIfCxx: CbBaseCxx{

    bool               _markAsSubChain = false; //// for elif else
    std::string        _cond;
    std::vector<CbIfCxx> _contBlock;

    CbIfCxx(bool isSubChain, std::string condtion);
    ~CbIfCxx() = default;

    CbIfCxx& addElif(std::string condition);
    std::string toString(int ident) override;


};

}

#endif //CPPWRITER_H
