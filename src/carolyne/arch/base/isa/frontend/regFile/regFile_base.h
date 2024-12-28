//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H
#define KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H

#include<unordered_map>
#include<string>
#include "carolyne/arch/base/util/regType.h"

namespace kathryn{
    namespace carolyne{

        struct ArchRegFile_Base{

            std::unordered_map<std::string,RegTypeMeta> _regMetas;

            bool isThereGroup(const std::string& groupName){
                return _regMetas.find(groupName) != _regMetas.end();
            }

            void addRegGroup(const std::string& groupName, int index_size, int reg_width){
                crlAss(!isThereGroup(groupName), "groupName " + groupName + " is duplicated");
                _regMetas.emplace(groupName, RegTypeMeta(index_size, reg_width));
                crlVb(
                "add " + groupName +
                " with amt " + std::to_string(_regMetas.at(groupName).AMT_REG) +
                " with width " + std::to_string(_regMetas.at(groupName).REG_WIDTH));
            }

            RegTypeMeta& getRegGroup(const std::string& groupName){
                crlAss(isThereGroup(groupName), "cannot find archGroup name : " + groupName + "<<--");
                return _regMetas.at(groupName);
            }

        };


    }



}

#endif //KATHRYN_SRC_CAROLYNE_ARCH_BASE_ISA_FRONTEND_REGFILE_REGFILE_BASE_H
