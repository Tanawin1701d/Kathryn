//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBIO_H
#define GLOBIO_H

namespace kathryn{


    enum GLOB_IO_TYPE{
        GLOB_IO_INPUT,
        GLOB_IO_OUTPUT,
        GLOB_IO_NOT_BOTH
    };

    class LogicGenBase;
    class GlobIo{
    protected:
        GLOB_IO_TYPE _globIoType = GLOB_IO_NOT_BOTH;
    public:
        virtual ~GlobIo() = default;
        void asInputGlob ();
        void asOutputGlob();
        virtual bool checkIntegrity() = 0;
        virtual LogicGenBase* getLogicGenFromGlobIo() = 0;
        GLOB_IO_TYPE getGlobIoType(){ return _globIoType;}

    };


}

#endif //GLOBIO_H
