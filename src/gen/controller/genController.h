//
// Created by tanawin on 20/6/2024.
//

#ifndef GENCONTROLLER_H
#define GENCONTROLLER_H
#include "frontEnd/cmd/paramReader.h"
#include "model/hwComponent/module/module.h"
#include "abstract/mainControlable.h"
#include "util/fileWriter/fileWriterBase.h"
#include "genStructure.h"

namespace kathryn{




    struct ModuleChecker;
    class GenController: public MainControlable{
    protected:
        const std::string pathToSynthesisRunner = "../synthesisRunner";
        const std::string pathToVivadoLaunch    = pathToSynthesisRunner + "/launchVivado.sh";

        const std::string _desPathParamPrefix = "genPath";
        const std::string _desSynthesisPrefix = "synName";
        std::string       _desVerilogPath;
        std::string       _desSynName;
        FileWriterBase*   _verilogWriter   = nullptr;
        Module*           _masterModule    = nullptr;
        ModuleGen*        _masterModuleGen = nullptr;
        GenStructure      _genStructure;




        void initGlobEle(bool isInput);

    public:

        explicit GenController() = default;

        virtual ~GenController() = default;

        void start() override;

        void initEnv(PARAM& param);

        void initEle();

        void routeIo();

        void genCefAll();

        void recruitModToGenSystem(); ////// it require bottom un generating
        ////// according to generation structure

        void generateEveryModule();


        /////// this is optional /////////////
        void startSynthesis();
        /////////////////////////////////////

        void reset() override;

        void clean() override;

        //////// module compare checker
    };



    GenController* getGenController();

}

#endif //GENCONTROLLER_H
