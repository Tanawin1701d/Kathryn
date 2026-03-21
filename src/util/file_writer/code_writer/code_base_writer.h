//
// Created by tanawin on 19/7/2024.
//

#ifndef CODEBASEWRITER_H
#define CODEBASEWRITER_H
#include "vector"
#include "string"
#include "cassert"
#include "iostream"
#include "ostream"

namespace kathryn{
    ////// code block statement

    ///// code block
    struct CbIf;
    struct CbBase{
        int last_order = 0;
        struct statement{
            std::string st;
            int         order;
            bool        ln;
            bool        is_comment;
        };
        std::vector<statement> _codeSt; ///// the simple code statement
        std::vector<int>         _sbOrder;   ////// the order of the statement
        std::vector<CbBase*>     _subBlocks;  ////// the sublock

        ///// constructor
        explicit CbBase(){};
        virtual ~CbBase();
        /////// add data + meta data section
        void add_st(std::string stm, bool ln = true);
        void add_cm(const std::string& comment); ///// comment
        void append_sub_block(CbBase* sub_block);
        ///// generate section
        virtual std::string to_string(int ident) = 0;
    };



}

#endif //CODEBASEWRITER_H
