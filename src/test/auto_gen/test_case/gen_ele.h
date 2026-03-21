//
// Created by tanawin on 26/6/2024.
//

#ifndef GENELE_H
#define GENELE_H
#include "front_end/cmd/param_reader.h"

namespace kathryn{
    class GenEle{
        int _id;
    public:
        explicit GenEle(int id);

        virtual ~GenEle() = default;

        void start_gen(PARAM& param);

        virtual void start(PARAM& param) = 0;

        int get_id() const{return _id;}

        bool operator < (const GenEle& rhs){
            return _id < rhs._id;
        }
    };
}
#endif //GENELE_H
