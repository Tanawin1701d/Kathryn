//
// Created by tanawin on 20/6/2024.
//

#ifndef GENCONTROLLER_H
#define GENCONTROLLER_H
#include "frontEnd/cmd/paramReader.h"
#include "model/hwComponent/module/module.h"

#include "abstract/mainControlable.h"

#include "util/fileWriter/fileWriterBase.h"

namespace kathryn{


    class GenController: public MainControlable{
    protected:
        const std::string _desPathParamPrefix = "genPath";
        std::string       _desVerilogPath;
        FileWriterBase*   _verilogWriter   = nullptr;
        Module*           _masterModule    = nullptr;
        ModuleGen*        _masterModuleGen = nullptr;

    public:

        explicit GenController() = default;

        void initEnv(PARAM& param);

        void start() override{};

        void initEle();

        void routeIo();

        void generateEveryModule();

        void reset() override;

        void clean() override;

    };





}

#endif //GENCONTROLLER_H
