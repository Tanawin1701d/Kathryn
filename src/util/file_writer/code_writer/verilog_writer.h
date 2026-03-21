//
// Created by tanawin on 27/11/25.
//

#ifndef SRC_UTIL_FILEWRITER_CODEWRITER_VERILOGWRITER_H
#define SRC_UTIL_FILEWRITER_CODEWRITER_VERILOGWRITER_H


#include "code_base_writer.h"
#include "cassert"

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
        virtual CbIfVerilog&     add_if(std::string condition);
        virtual CbAlwaysVerilog& add_always(Verilog_SEN_TYPE ver_sen_type, std::string sen_name);
        virtual CbSwitchVerilog& add_switch(std::string switch_ident);
        virtual CbBaseVerilog&   add_sub_block();
        std::string to_string(int ident) override;
    };


    struct CbIfVerilog: CbBaseVerilog{
        bool               _markAsSubChain = false; //// for elif else
        std::string        _cond;
        std::vector<CbIfVerilog*> _contBlock;

        CbIfVerilog(bool is_sub_chain, std::string condtion);
        ~CbIfVerilog() override;

        CbIfVerilog& add_elif(std::string condition);
        std::string to_string(int ident) override;

        //////// disable the uncodable block
        CbAlwaysVerilog& add_always(Verilog_SEN_TYPE ver_sen_type, std::string sen_name) override{assert(false);}
        ////////////////////////////////////

    };

    struct CbAlwaysVerilog: CbBaseVerilog{
              Verilog_SEN_TYPE _senType = VLST_CNT;
              std::string      _senName;
        const std::string      Verilog_SEN_TYPE_STR[3] = {"posedge", "negedge", "*"};


        CbAlwaysVerilog(Verilog_SEN_TYPE sen_type, std::string  sen_name);
        ~CbAlwaysVerilog() = default;


        std::string to_string(int ident) override;
    };

    struct CbSwitchVerilog: CbBaseVerilog{
        std::string _switchIdent;
        bool is_default_occure = false;
        std::vector<int> _caseIdents;

        CbSwitchVerilog(std::string switch_ident):
                _switchIdent(std::move(switch_ident)){}

        /**
         * disable uncodable block
         */
        CbIfVerilog&     add_if      (std::string condition)                            override{assert(false);}
        CbAlwaysVerilog& add_always  (Verilog_SEN_TYPE ver_sen_type, std::string sen_name) override{assert(false);}
        CbSwitchVerilog& add_switch  (std::string switch_ident)                          override{assert(false);}
        CbBaseVerilog&   add_sub_block()                                                 override{assert(false);}

        CbBaseVerilog& add_case(int case_val);
        std::string    to_string(int ident) override;

    };

}

#endif //SRC_UTIL_FILEWRITER_CODEWRITER_VERILOGWRITER_H