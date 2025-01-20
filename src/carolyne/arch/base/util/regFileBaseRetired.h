//
// Created by tanawin on 30/12/2024.
//

#ifndef src_carolyne_arch_base_util_REGFILEBASE_H
#define src_carolyne_arch_base_util_REGFILEBASE_H

#include<unordered_map>
#include<string>
#include "regFileType.h"
#include "tableMeta.h"


    // namespace kathryn::carolyne{
    //
    //     struct RegFileBase{
    //         std::unordered_map<std::string,RegTypeMeta> _regMetas;
    //         virtual ~RegFileBase()= default;
    //         bool isThereGroup(const std::string& groupName){
    //             return _regMetas.find(groupName) != _regMetas.end();
    //         }
    //         void addRegGroup(const std::string& groupName, int index_size, int reg_width){
    //             crlAss(!isThereGroup(groupName), "groupName " + groupName + " is duplicated");
    //             _regMetas.emplace(groupName, RegTypeMeta(index_size, reg_width));
    //             crlVb(
    //             "add " + groupName +
    //             " with amt " + std::to_string(_regMetas.at(groupName).AMT_REG) +
    //             " with width " + std::to_string(_regMetas.at(groupName).REG_WIDTH));
    //         }
    //
    //         RegTypeMeta& getRegTypeMetaGroup(const std::string& groupName){
    //             crlAss(isThereGroup(groupName), "cannot find archGroup name : " + groupName + "<<--");
    //             return _regMetas.at(groupName);
    //         }
    //     };
    // }


#endif //src_carolyne_arch_base_util_REGFILEBASE_H
