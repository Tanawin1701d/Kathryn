//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBIO_H
#define GLOBIO_H
#include <string>

namespace kathryn{


    enum WIRE_MARKER_TYPE{
        WMT_GLOB_INPUT, ///// act as global input
        WMT_GLOB_OUTPUT, ///// act as global output
        WMT_INPUT_MD, ///// act as input of the current module
        WMT_OUTPUT_MD, ////// act as output of the current module
        WMT_BCI, /////// declare that this wire can be broadcast
        WMT_NONE
    };

    class WireAuto;
    class Operable;

    class WireMarker{
    protected:
        std::string ioValue = "";
        WIRE_MARKER_TYPE _marker = WMT_NONE;
    public:
        virtual ~WireMarker() = default;
        void asInputGlob (std::string value = "");
        void asInput     (std::string value = "");
        void asOutputGlob(std::string value = "");
        void asOutput    (std::string value = "");
        void setIoName   (const std::string&);
        virtual std::string  getGlobIoName();
        virtual bool checkIntegrity       ()                 = 0;
        virtual void connectToThisIo      (WireAuto* wireIo) = 0;
        virtual Operable* getOprFromGlobIo()                 = 0;
        WIRE_MARKER_TYPE getMarker        (){ return _marker;}

    };


}

#endif //GLOBIO_H
