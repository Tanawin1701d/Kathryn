//
// Created by tanawin on 20/6/2024.
//

#ifndef GENCONTROLLER_H
#define GENCONTROLLER_H
#include "params/prjParam.h"
#include "frontEnd/cmd/paramReader.h"
#include "model/hwComponent/module/module.h"
#include "abstract/mainControlable.h"
#include "util/fileWriter/fileWriterBase.h"
#include "genStructure.h"
#include "util/fileWriter/fileWriterGroup.h"
#include "util/str/strUtil.h"

namespace kathryn{




    struct ModuleChecker;
    class GenController: public MainControlable{
    protected:
        const std::string PROJECT_PATH          = "..";
        const std::string pathToSynthesisRunner = PROJECT_PATH + "/synthesisRunner";
        const std::string pathToVivadoLaunch    = pathToSynthesisRunner + "/launchVivado.sh";

        const std::string _desVerilogFolderParamPrefix      = "genFolder";
        const std::string _desVerilogTopFileNameParamPrefix = "topFileName";
        const std::string _desVerilogTopModNameParamPrefix  = "topModName";
        const std::string _desVerilogExtractParamPrefix     = "extractMulFile";
        const std::string _desSynthesisPrefix = "synName";
        ////// get from parameter file
        const std::string _file_suffix = ".v";
        std::string       _desVerilogFolder;
        std::string       _desVerilogTopFileName;
        std::string       _desVerilogTopModName;
        bool              _extractMulFile = false;
        std::string       _desSynName;
        /////// working element
        FileWriterGroup   _writerGroup;
        Module*           _masterModule    = nullptr;
        ModuleGen*        _masterModuleGen = nullptr;




        void initGlobEle(bool isInput);

    public:

        explicit GenController();

        virtual ~GenController() = default;

        /** generation have 5 major steps
         * 1. initEle() recruit model and create its necessary meta-data
         * 2. routeIo() route the io that to the collect place
         *  ///// abandon we do not maintain this anymore 3. genCefAll() gen meta data for compare to reduce data redundancy
         * 4. recruitModToGenSystem() start compare model to reduce redundancy
         * 5. generateEveryModule() dump it to verilog
         */
        void start() override;

        void initEnv(PARAM& param);

        void initEle();

        void routeIo();

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
