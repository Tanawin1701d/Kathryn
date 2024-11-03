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
#include "util/fileWriter/fileWriterGroup.h"

namespace kathryn{




    struct ModuleChecker;
    class GenController: public MainControlable{
    protected:
        const std::string pathToSynthesisRunner = "../synthesisRunner";
        const std::string pathToVivadoLaunch    = pathToSynthesisRunner + "/launchVivado.sh";

        const std::string _desVerilogFolderParamPrefix      = "genFolder";
        const std::string _desVerilogTopFileNameParamPrefix = "topFileName";
        const std::string _desVerilogTopModNameParamPrefix  = "topModName";
        const std::string _desVerilogExtractParamPrefix     = "extractMulFile";
        const std::string _desSynthesisPrefix = "synName";
        ////// get from parameter file
        const std::string _FILE_SUFFIX = ".v";
        std::string       _desVerilogFolder;
        std::string       _desVerilogTopFileName;
        std::string       _desVerilogTopModName;
        bool              _extractMulFile = false;
        std::string       _desSynName;
        /////// working element
        FileWriterGroup   _writerGroup;
        Module*           _masterModule    = nullptr;
        ModuleGen*        _masterModuleGen = nullptr;
        GenStructure      _genStructure;




        void initGlobEle(bool isInput);

    public:

        explicit GenController() = default;

        virtual ~GenController() = default;

        /** generation have 5 major steps
         * 1. initEle() recruit model and create its necessary meta-data
         * 2. routeIo() route the io that to the collect place
         * 3. genCefAll() gen meta data for compare to reduce data redundancy
         * 4. recruitModToGenSystem() start compare model to reduce redundancy
         * 5. generateEveryModule() dump it to verilog
         */
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
