//
// Created by tanawin on 24/3/2567.
//

#ifndef KATHRYN_PIPEMNG_H
#define KATHRYN_PIPEMNG_H

#include <utility>
#include <map>
#include "model/hwComponent/expression/expression.h"
#include "abstract/mainControlable.h"
#include "model/hwComponent/value/value.h"

namespace kathryn{


    typedef unsigned long long PIPID;

    class Pipe{
    public:
        PIPID       _pipeId            = -1;
        Val*        _dummyStart        = nullptr;
        Val*        _dummyStop         = nullptr;
        expression* _masterReadyToSend = nullptr; /////// B read to check that it is recieveable, A write notify that data is ready
        expression* _slaveReadyToRecv  = nullptr; /////// A read to check that other side is get data, B write to notfy that data is accept

        explicit Pipe(PIPID pipeId);
        Pipe(const Pipe& rhs);
        Pipe& operator = (const Pipe& rhs);
        void setDummyStartPipe();
        void setDummyStopPipe();


    };


    class PipeController: MainControlable{

    std::vector<Pipe*> _pipeMeta; /////// for user and s

    public:
        void start() override{};

        void reset() override;

        void clean() override;

        Pipe& createPipe();

        [[nodiscard]]
        PIPID getNextPipeId() const {return _pipeMeta.size();}

    };

    Pipe& makePipe();
    std::vector<Pipe*> makePipes(int amt);

}

#endif //KATHRYN_PIPEMNG_H
