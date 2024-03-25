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


    typedef unsigned long long pipId;


    class Pipe{
    public:
        bool        _isAlloc            = false;
        pipId       _pipeId             = -1;
        Val*        _dummyVal           = nullptr;
        expression* _availSendSignal    = nullptr;
        expression* _notifyToSendSignal = nullptr;

        explicit Pipe(bool allocRequire = true);
        Pipe(const Pipe& rhs);
        Pipe& operator = (const Pipe& rhs);

        /** to allocate expression that is used to communication between block*/
        void alloc();
        bool isAlloc() const;
        /** reverese for destination block perspective*/
        void reverse();
        void setAsDummyPipe();


    };






    class PipeController: MainControlable{

    std::vector<Pipe> _pipeMeta; /////// for user and s

    public:
        void start() override{};

        void reset() override;

        void clean() override;

        pipId allocPipe(Pipe* newPipeCom);

        [[nodiscard]]
        pipId getNextPipeId() const {return _pipeMeta.size();}

    };





}

#endif //KATHRYN_PIPEMNG_H
