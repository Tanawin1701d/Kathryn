//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBIO_H
#define GLOBIO_H
#include <string>

namespace kathryn{


    enum GLOB_IO_TYPE{
        GLOB_IO_INPUT,
        GLOB_IO_OUTPUT,
        GLOB_IO_BCI,
        GLOB_IO_NOT_BOTH
    };

    class WireIo;
    class Operable;

    class GlobIoItf{
    protected:
        std::string ioValue = "";
        GLOB_IO_TYPE _globIoType = GLOB_IO_NOT_BOTH;
    public:
        virtual ~GlobIoItf() = default;
        void asInputGlob (std::string value = "");
        void asOutputGlob(std::string value = "");
        void setIoName(const std::string&);
        virtual void asBci();
        virtual std::string  getGlobIoName();
        virtual bool         checkIntegrity()                    = 0;
        virtual void         connectToThisIo(WireIo* wireIo) = 0;
        virtual Operable*    getOprFromGlobIo()             = 0;
        GLOB_IO_TYPE         getGlobIoType(){ return _globIoType;}

    };


}

#endif //GLOBIO_H
