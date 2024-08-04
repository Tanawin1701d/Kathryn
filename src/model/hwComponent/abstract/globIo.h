//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBIO_H
#define GLOBIO_H
#include <string>

namespace kathryn{


    enum GLOB_IO_TYPE{
        GLOB_IO_INPUT, ///// act as global input
        GLOB_IO_OUTPUT, ///// act as global output
        GLOB_IO_INPUT_MD, ///// act as input of the current module
        GLOB_IO_OUTPUT_MD, ////// act as output of the current module
        GLOB_IO_BCI, /////// declare that this wire can be broadcast
        GLOB_IO_NOT_BOTH
    };

    class WireAutoGen;
    class Operable;

    class GlobIo{
    protected:
        std::string ioValue = "";
        GLOB_IO_TYPE _ioType = GLOB_IO_NOT_BOTH;
    public:
        virtual ~GlobIo() = default;
        void asInputGlob (std::string value = "");
        void asInput     (std::string value = "");
        void asOutputGlob(std::string value = "");
        void asOutput    (std::string value = "");
        void setIoName(const std::string&);
        virtual std::string  getGlobIoName();
        virtual bool checkIntegrity()                = 0;
        virtual void connectToThisIo(WireAutoGen* wireIo) = 0;
        virtual Operable* getOprFromGlobIo()         = 0;
        GLOB_IO_TYPE getGlobIoType(){ return _ioType;}

    };


}

#endif //GLOBIO_H
