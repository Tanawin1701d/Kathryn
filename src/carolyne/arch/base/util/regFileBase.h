//
// Created by tanawin on 30/12/2024.
//

#ifndef src_carolyne_arch_base_util_REGFILEBASE_H
#define src_carolyne_arch_base_util_REGFILEBASE_H

#include<unordered_map>
#include<string>
#include "regType.h"
#include "tableMeta.h"

namespace kathryn{
    namespace carolyne{
        struct RegFileBase{

            std::unordered_map<std::string,RegTypeMeta> _regMetas;
            std::unordered_map<std::string, TableMeta> _regTableMetas;

            virtual ~RegFileBase(){}

            bool isThereGroup(const std::string& groupName){
                return _regMetas.find(groupName) != _regMetas.end();
            }

            bool isThereTable(const std::string& groupName){
                return _regTableMetas.find(groupName) != _regTableMetas.end();
            }

            void addRegGroup(const std::string& groupName, int index_size, int reg_width){
                crlAss(!isThereGroup(groupName), "groupName " + groupName + " is duplicated");
                _regMetas.emplace(groupName, RegTypeMeta(index_size, reg_width));
                crlVb(
                "add " + groupName +
                " with amt " + std::to_string(_regMetas.at(groupName).AMT_REG) +
                " with width " + std::to_string(_regMetas.at(groupName).REG_WIDTH));
            }

            virtual  RowMeta buildPhyRegRowMeta(RegTypeMeta& regTypeMeta){
                crlAss(false, "rowMeta is not implemented");
            }

            virtual void buildPhyRegTableMeta(){
                /** build table meta for physical regfile */
                for (auto& regMeta: _regMetas){
                    std::string   name = regMeta.first;
                    RegTypeMeta&  regTypeMeta = regMeta.second;

                    RowMeta   rm = buildPhyRegRowMeta(regTypeMeta);
                    TableMeta tm(regTypeMeta.getIndexWidth(), rm);

                    _regTableMetas.insert({name, tm});
                }
            }

            RegTypeMeta& getRegTypeMetaGroup(const std::string& groupName){
                crlAss(isThereGroup(groupName), "cannot find archGroup name : " + groupName + "<<--");
                return _regMetas.at(groupName);
            }

            TableMeta& getRegTableMetaGroup(const std::string& groupName){
                crlAss(isThereGroup(groupName), "cannot find table for group : " + groupName + "<<<----");
                return _regTableMetas.at(groupName);
            }


        };
    }
}

#endif //src_carolyne_arch_base_util_REGFILEBASE_H
