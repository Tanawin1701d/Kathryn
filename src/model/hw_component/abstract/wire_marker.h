//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBIO_H
#define GLOBIO_H
#include "string"

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
        std::string io_value = "";
        WIRE_MARKER_TYPE _marker = WMT_NONE;
    public:
        virtual ~WireMarker() = default;

        /** generator must have global input for this signal*/
        void as_input_glob (std::string value = "");
        /** mark that it is input io for the module*/
        void as_input     (std::string value = "def_input_name");
        /** generator must have global output for this signal*/
        void as_output_glob(std::string value = "");
        /** mark that it is output io for the module*/
        void as_output    (std::string value = "def_output_name");
        void set_io_name   (const std::string&);
        virtual std::string  get_glob_io_name();
        virtual bool         check_integrity()       = 0;
        virtual Operable*get_opr_from_glob_io_ptr()     = 0;
        virtual Assignable*get_asb_from_wire_marker_ptr() = 0;
        WIRE_MARKER_TYPE get_marker        (){ return _marker;}

    };


}

#endif //GLOBIO_H
