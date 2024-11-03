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
    class Assignable;

    /** this class is used to be the marker that
     *  wire or reg is special for io input output or other special
     */

    class WireMarker{
    protected:
        std::string ioValue = "";
        WIRE_MARKER_TYPE _marker = WMT_NONE;
    public:
        virtual ~WireMarker() = default;

        /** generator must have global input for this signal*/
        void asInputGlob (std::string value = "");
        /** mark that it is input io for the module*/
        void asInput     (std::string value = "defInputName");
        /** generator must have global output for this signal*/
        void asOutputGlob(std::string value = "");
        /** mark that it is output io for the module*/
        void asOutput    (std::string value = "defOutputName");
        void setIoName   (const std::string&);
        virtual std::string  getGlobIoName();
        virtual bool         checkIntegrity()       = 0;
        virtual Operable*    getOprFromGlobIo()     = 0;
        virtual Assignable*  getAsbFromWireMarker() = 0;
        WIRE_MARKER_TYPE getMarker        (){ return _marker;}

    };


}

#endif //GLOBIO_H
