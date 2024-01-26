//
// Created by tanawin on 18/1/2567.
//

#include "modelSimEngine.h"



namespace kathryn{

    /**
     * rtl simEngine
     * */

    RtlSimEngine::RtlSimEngine(int sz, VCD_SIG_TYPE sigType):
            _isSimMetaSet(false),
            _sigType(sigType),
            _simMeta(RTL_Meta_afterMf()),
            _backVal(sz),
            _curVal(sz)
            {}

    void RtlSimEngine::declareSimVar() {
        assert(_isSimMetaSet);
        if (_simMeta._recCmd){
            _simMeta._writer->addNewVar(_sigType,
                                        _simMeta._recName,
                                        {0,_curVal.getLen()}
                                        );
        }
    }

    void RtlSimEngine::tryWriteValue() {
        assert(_isSimMetaSet);
        if (_simMeta._recCmd){
            /** request for record*/
            assert(_simMeta._writer != nullptr);
            _simMeta._writer->addNewValue(_simMeta._recName, _curVal);
        }
    }


    void FlowSimEngine::incUsedTime() {
        amtUsed++;
    }
}

