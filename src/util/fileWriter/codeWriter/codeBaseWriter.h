//
// Created by tanawin on 19/7/2024.
//

#ifndef CODEBASEWRITER_H
#define CODEBASEWRITER_H
#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <ostream>

namespace kathryn{
    ////// code block statement

    ///// code block
    struct CbIf;
    struct CbBase{
        int lastOrder = 0;
        struct statement{
            std::string st;
            int         order;
            bool        ln;
            bool        isComment;
        };
        std::vector<statement> _codeSt; ///// the simple code statement
        std::vector<int>         _sbOrder;   ////// the order of the statement
        std::vector<CbBase*>     _subBlocks;  ////// the sublock

        ///// constructor
        explicit CbBase(){};
        virtual ~CbBase();
        /////// add data + meta data section
        void addSt(std::string stm, bool ln = true);
        void addCm(const std::string& comment); ///// comment
        void appendSubBlock(CbBase* subBlock);
        ///// generate section
        virtual std::string toString(int ident) = 0;
    };



}

#endif //CODEBASEWRITER_H
