//
// Created by tanawin on 26/6/2024.
//

#ifndef GENELE_H
#define GENELE_H
#include "frontEnd/cmd/paramReader.h"

namespace kathryn{
    class GenEle{
        int _id;
    public:
        explicit GenEle(int id);

        virtual ~GenEle() = default;

        void startGen(const PARAM& param);

        virtual void start(const PARAM& param) = 0;

        int getId() const{return _id;}

        bool operator < (const GenEle& rhs){
            return _id < rhs._id;
        }
    };
}
#endif //GENELE_H
