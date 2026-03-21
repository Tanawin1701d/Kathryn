//
// Created by tanawin on 19/7/2024.
//

#ifndef CPPWRITER_H
#define CPPWRITER_H
#include "code_base_writer.h"
#include "cassert"


namespace kathryn{

    constexpr int CXX_IDENT = 4;


struct CbIfCxx;
struct CbSwitchCxx;
struct CbFuncDec;

constexpr char CXX_ULL_SUFFIX [] = "ULL";

struct CbBaseCxx: CbBase{

    CbBaseCxx(): CbBase(){}
    ~CbBaseCxx()  = default;
    virtual CbIfCxx&     add_if(std::string condition);
    virtual CbSwitchCxx& add_switch(std::string switch_ident);
    virtual CbBaseCxx&   add_sub_block();
    std::string          to_string(int ident) override;


};


struct CbIfCxx: CbBaseCxx{

    bool               _markAsSubChain = false; //// for elif else
    std::string        _cond;
    std::vector<CbIfCxx*> _contBlock;

    CbIfCxx(bool is_sub_chain, std::string condtion);
    ~CbIfCxx(){
        for(CbIfCxx* cont_block: _contBlock){
            delete cont_block;
        }
    }

    CbIfCxx& add_elif(std::string condition);
    std::string to_string(int ident) override;

};

struct CbSwitchCxx: CbBaseCxx{
    std::string _switchIdent;
    bool is_default_occure = false;
    std::vector<int> _caseIdents;

    CbSwitchCxx(std::string switch_ident):
            _switchIdent(std::move(switch_ident)){}

    /////// disable the unused function
    CbIfCxx&     add_if(std::string condition)       override{assert(false);}
    CbSwitchCxx& add_switch(std::string switch_ident) override{assert(false);}
    CbBaseCxx&   add_sub_block() override{assert(false);}

    CbBaseCxx&  add_case(int case_val);
    std::string to_string(int ident) override;

};

}

#endif //CPPWRITER_H
