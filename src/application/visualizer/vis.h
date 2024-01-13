//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_VIS_H
#define KATHRYN_VIS_H
//
//#include <string>
//
//#include "model/hwComponent/module/module.h"
//#include "model/util/StateRegUtil.h"
//
//namespace kathryn{
//
//    /***
//     * dependState recursively get until got state reg or register
//     * */
//    struct StateMeta{
//        std::string stateName;
//        std::vector<StResMeta> metas;
//
//        std::string getDebugString(){
//            std::string ret = stateName + " : \n";
//            for (auto& meta: metas){
//                ret += "        " + meta.getDebugString() + "\n";
//            }
//            return ret;
//        }
//
//    };
//
//    /**
//     * metadata for exprMetas , normal reg, normal wire not recursively get value
//     * */
//    struct CompDebugMessage{
//        std::string compName;
//        std::vector<std::string> values;
//        std::string getDebugString() {
//            std::string ret = compName + " : \n";
//            for (const auto& value: values) {
//                ret += "       " + value + "\n";
//            }
//            return ret;
//        }
//    };
//
//    /** Vis is used for print out state and
//     * condition that occure in each module
//     * */
//    class Vis{
//
//    private:
//        Module* _sampleModule;
//        std::vector<StateMeta> stateMetas;
//        std::vector<CompDebugMessage> regMetas;
//        std::vector<CompDebugMessage> wireMetas;
//        std::vector<CompDebugMessage> exprMetas;
//
//        bool retrieveYet = false;
//
//    protected:
//        template<typename T>
//        void retrieveSimpleAsm(T& srcHwComp, std::vector<CompDebugMessage>& desStore){
//            for (auto hwCompPtr: srcHwComp){
//                CompDebugMessage dbgMsg;
//                dbgMsg.compName = hwCompPtr->getGlobalName();
//                dbgMsg.values = hwCompPtr->getDebugAssignmentValue();
//                desStore.push_back(dbgMsg);
//            }
//        }
//
//        void retrieveStateReg();
//
//    public:
//        explicit Vis(Module* sampleModel);
//
//        void execute();
//
//        void print();
//
//        static void printSimpleAsm(std::vector<CompDebugMessage>& dbgMsgs){
//            for (auto& meta: dbgMsgs){
//                std::cout << meta.getDebugString();
//                std::cout << "----------\n";
//            }
//        }
//
//    };
//
//
//
//}

#endif //KATHRYN_VIS_H
