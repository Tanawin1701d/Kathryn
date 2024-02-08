//
// Created by tanawin on 18/1/2567.
//

#include "modelSimEngine.h"



namespace kathryn{

    /**
     * rtl simEngine
     * */

    RtlSimEngine::RtlSimEngine(int sz, VCD_SIG_TYPE sigType, bool simForNext):
            _simForNext(simForNext),
            _isSimMetaSet(false),
            _sigType(sigType),
            _simMeta(RTL_Meta_afterMf()),
            _sz(sz),
            _isCurValSim(simForNext), /***typically reg is true but wire is false*/
            _isNextValSim(false),
            _curVal(sz),
            _nextVal(sz)
            {
                assert(sz > 0);
            }

    void RtlSimEngine::declareSimVar() {
        assert(_isSimMetaSet);
        if (_simMeta._recCmd){
            _simMeta._writer->addNewVar(_sigType,
                                        _simMeta._recName,
                                        {0,_curVal.getLen()}
                                        );
//            _simMeta._writer->addNewVar(_sigType,
//                                        _simMeta._recName + "_BACK",
//                                        {0,_curVal.getLen()}
//            );



        }
    }

    void RtlSimEngine::tryWriteValue() {
        assert(_isSimMetaSet);
        if (_simMeta._recCmd){
            /** request for record*/
            assert(_simMeta._writer != nullptr);
            _simMeta._writer->addNewValue(_simMeta._recName, _curVal);
            //_simMeta._writer->addNewValue(_simMeta._recName + "_BACK", _backVal);
        }
    }


    /**
     * flow sim engine
     * */


    void FlowSimEngine::incUsedTime() {
        amtUsed++;
    }

    void FlowSimEngine::finalizeData(){
        assert(_meta._writer != nullptr);
        _meta._writer->freq = amtUsed;
        _meta._writer->localName = _meta._recName;
    }
}

