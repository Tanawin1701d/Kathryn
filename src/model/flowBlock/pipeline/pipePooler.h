//
// Created by tanawin on 2/3/2025.
//

#ifndef SRC_MODEL_FLOWBLOCK_PIPELINE_PIPEPOOLER_H
#define SRC_MODEL_FLOWBLOCK_PIPELINE_PIPEPOOLER_H
#include <string>
#include "unordered_map"

#include "abstract/mainControlable.h"
#include "pipe.h"
#include "pipeTran.h"


namespace kathryn{


    class PipePooler: public MainControlable{
    public:
        std::unordered_map<std::string, FlowBlockPipeBase* > _pipeBlks; //// pipe block name
        std::unordered_map<std::string, FlowBlockPipeTran* > _tranBlks; //// targetName

        //////// pipe Block Management

        bool isTherePipe(const std::string& pipeName);
        bool isThereTran(const std::string& tranName);

        void addPipeBlk(FlowBlockPipeBase* pipeBlk);
        void addTranBlk(FlowBlockPipeTran* pipeBlk);

        Operable* getPipeReadySignal(const std::string& pipeName);
        Operable* getTranReadySignal(const std::string& targetPipeName);

        //////// controller manager

        void start() override;
        void reset() override;
        void clean() override;

    };

    PipePooler* getPipePooler();


}


#endif //SRC_MODEL_FLOWBLOCK_PIPELINE_PIPEPOOLER_H
