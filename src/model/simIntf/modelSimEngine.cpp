//
// Created by tanawin on 18/1/2567.
//

#include "modelSimEngine.h"



namespace kathryn{

    /**
     * flow sim engine
     * */


    void FlowSimEngine::incUsedTime() {
        amtUsed++;
        /**for node or other block that did not assign writer
         * when beforePrepare was invoked
         * , we skip it
         * */
        if (_isSimMetaSet){
            assert(_meta._writer != nullptr);
            _meta._writer->freq++;
        }

    }

}

