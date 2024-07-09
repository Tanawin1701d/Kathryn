//
// Created by tanawin on 9/7/2024.
//

#include "instrBase.h"
#include "util/str/strUtil.h"


namespace kathryn{

//////////////////////////

    OPR_META::OPR_META(int archSize, int idxSize, int regNo):
    data(makeOprReg("regData" + std::to_string(regNo), archSize)),
    idx(makeOprReg("regIdx" + std::to_string(regNo),idxSize)),
    valid(makeOprReg("regValid" + std::to_string(regNo), 1)){
        assert(1 << idxSize >= archSize);
        assert(regNo > 0);
    }


/////////////////////////

InstrRepo::InstrRepo(int instrSize):
    decodeLock(false),
    INSTR_SIZE(instrSize){}

void InstrRepo::addRule(const std::string& rule){
    assert(!decodeLock);
    rules.push_back(rule);
}

void InstrRepo::startTokenize(){

    decodeLock = true;
    for (const std::string& rule: rules){
        int curIdx = INSTR_SIZE;
        std::stack<token> tokenSt;
        std::vector<token> tokenResult;
        /////// iterate all in string
        for (int idx = 0; idx < rule.size(); idx--){
            char bitChar = rule[idx];
            switch (bitChar){
                case '<':{
                            tokenSt.push({{-1, curIdx}});
                            break;
                          }
                case '>':{
                            token topToken = tokenSt.top();
                            topToken.finalToken();
                            tokenResult.push_back(topToken);
                            tokenSt.pop();
                            curIdx -= topToken.sl.getSize();
                            break;
                         }
                default :{
                    assert(!tokenSt.empty());
                    tokenSt.top().value += bitChar; break;
                }
            }
        }
        if (curIdx != 0){
            std::cout << "[warning] instruction tokenizing not complete" << std::endl;
        }
        tokenizedRules.push_back(tokenResult);
    }

}


}
