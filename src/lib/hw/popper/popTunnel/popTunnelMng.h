//
// Created by tanawin on 26/1/2025.
//

#ifndef src_lib_hw_popper_POPTUNNELMNG_H
#define src_lib_hw_popper_POPTUNNELMNG_H
#include <map>
#include <string>


#include "model/debugger/modelDebugger.h"

#include "popHST.h"
#include "popBCT.h"

namespace kathryn{

    struct PopTunnelMng{

        std::map<std::string, PopHST*> popHSTMap{};
        std::map<std::string, PopBCT*> popBCTMap{};


        ~PopTunnelMng(){
            for (auto hstPair: popHSTMap){delete hstPair.second;}
            for (auto bctPair: popBCTMap){delete bctPair.second;}
        }

        ////////////// hand shake management
        [[nodiscard]]
        bool isThereHST(const std::string& name){
            return popHSTMap.find(name) != popHSTMap.end();
        }

        [[nodiscard]]
        PopHST& getHST(const std::string& name){
            mfAssert(isThereHST(name), "there is no hand shake tunnel : " + name);
            return *popHSTMap[name];
        }

        [[nodiscard]]
        PopHST& createHST(const std::string& name, const RowMeta& sendRowMeta, const RowMeta&  recvRowMeta = {}){
            mfAssert(!isThereHST(name), "there is identicle hand shake tunnel : " + name);
            popHSTMap.insert({name, new PopHST(name, sendRowMeta, recvRowMeta)});
            return *popHSTMap[name];
        }

        ////////////// bc management
        bool isThereBCT(const std::string& name){
            return popBCTMap.find(name) != popBCTMap.end();
        }

        [[nodiscard]]
        PopBCT& getBCT(const std::string& name){
            mfAssert(isThereBCT(name), "there is no hand shake tunnel : " + name);
            return *popBCTMap[name];
        }

        [[nodiscard]]
        PopBCT& createBCT(const std::string& name, const RowMeta& sendRowMeta, const RowMeta&  recvRowMeta = {}){
            mfAssert(!isThereBCT(name), "there is identicle hand shake tunnel : " + name);
            popBCTMap.insert({name, new PopBCT(name, sendRowMeta)});
            return *popBCTMap[name];
        }

    };

    struct PopTunnelable{

        PopTunnelMng* _tunnelMng = nullptr;

        explicit PopTunnelable(PopTunnelMng* tunnelMng = nullptr):
        _tunnelMng(tunnelMng){}

        /** HAND SHAKE TUNNEL*/
        [[nodiscard]] bool    isThereHST(const std::string& name) const{ return _tunnelMng->isThereHST(name);}
        [[nodiscard]] PopHST& getHST(const std::string& name) const{  return _tunnelMng->getHST(name);    }
        [[nodiscard]] PopHST& createHST(const std::string& name, const RowMeta& sendRowMeta, const RowMeta&  recvRowMeta = {}) const{
            return _tunnelMng->createHST(name, sendRowMeta, recvRowMeta);
        }
        /** POP TUNNEL*/
        [[nodiscard]] bool    isThereBCT(const std::string& name) const{ return _tunnelMng->isThereBCT(name);}
        [[nodiscard]] PopBCT& getBCT    (const std::string& name) const { return _tunnelMng->getBCT(name);    }
        [[nodiscard]] PopBCT& createBCT (const std::string& name, const RowMeta& sendRowMeta, const RowMeta&  recvRowMeta = {}) const{
            return _tunnelMng->createBCT(name, sendRowMeta, recvRowMeta);
        }

    };

}

#endif //src_lib_hw_popper_POPTUNNELMNG_H
