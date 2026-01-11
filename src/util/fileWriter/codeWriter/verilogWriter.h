//
// Created by tanawin on 27/11/25.
//

#ifndef SRC_UTIL_FILEWRITER_CODEWRITER_VERILOGWRITER_H
#define SRC_UTIL_FILEWRITER_CODEWRITER_VERILOGWRITER_H


#include "codeBaseWriter.h"
#include <cassert>

namespace kathryn{

    constexpr int Verilog_IDENT = 4;

    enum Verilog_SEN_TYPE{
        VLST_POSEDGE = 0,
        VLST_NEGEDGE = 1,
        VLST_ALWAYS  = 2,
        VLST_CNT     = 3
    };

    struct CbIfVerilog;
    struct CbAlwaysVerilog;
    struct CbSwitchVerilog;

    struct CbBaseVerilog: CbBase{
        CbBaseVerilog(): CbBase(){}
        ~CbBaseVerilog()  = default;
        virtual CbIfVerilog&     addIf(std::string condition);
        virtual CbAlwaysVerilog& addAlways(Verilog_SEN_TYPE verSenType, std::string senName);
        virtual CbSwitchVerilog& addSwitch(std::string switchIdent);
        virtual CbBaseVerilog&   addSubBlock();
        std::string toString(int ident) override;
    };


    struct CbIfVerilog: CbBaseVerilog{
        bool               _markAsSubChain = false; //// for elif else
        std::string        _cond;
        std::vector<CbIfVerilog> _contBlock;

        CbIfVerilog(bool isSubChain, std::string condtion);
        ~CbIfVerilog() = default;

        CbIfVerilog& addElif(std::string condition);
        std::string toString(int ident) override;

        //////// disable the uncodable block
        CbAlwaysVerilog& addAlways(Verilog_SEN_TYPE verSenType, std::string senName) override{assert(false);}
        ////////////////////////////////////

    };

    struct CbAlwaysVerilog: CbBaseVerilog{
              Verilog_SEN_TYPE _senType = VLST_CNT;
              std::string      _senName;
        const std::string      Verilog_SEN_TYPE_STR[3] = {"posedge", "negedge", "*"};


        CbAlwaysVerilog(Verilog_SEN_TYPE senType, std::string  senName);
        ~CbAlwaysVerilog() = default;


        std::string toString(int ident) override;
    };

    struct CbSwitchVerilog: CbBaseVerilog{
        std::string _switchIdent;
        bool isDefaultOccure = false;
        std::vector<int> _caseIdents;

        CbSwitchVerilog(std::string switchIdent):
                _switchIdent(std::move(switchIdent)){}

        /**
         * disable uncodable block
         */
        CbIfVerilog&     addIf      (std::string condition)                            override{assert(false);}
        CbAlwaysVerilog& addAlways  (Verilog_SEN_TYPE verSenType, std::string senName) override{assert(false);}
        CbSwitchVerilog& addSwitch  (std::string switchIdent)                          override{assert(false);}
        CbBaseVerilog&   addSubBlock()                                                 override{assert(false);}

        CbBaseVerilog& addCase(int caseVal);
        std::string    toString(int ident) override;

    };

}

#endif //SRC_UTIL_FILEWRITER_CODEWRITER_VERILOGWRITER_H