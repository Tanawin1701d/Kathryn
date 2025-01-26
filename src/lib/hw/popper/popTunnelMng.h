//
// Created by tanawin on 26/1/2025.
//

#ifndef src_lib_hw_popper_POPTUNNELMNG_H
#define src_lib_hw_popper_POPTUNNELMNG_H
#include <map>
#include <string>
#include "model/debugger/modelDebugger.h"
#include "popTunnel.h"

namespace kathryn{

    struct PopTunnelMng{

        std::map<std::string, PopTunnel*> popTunnelMap;


        PopTunnel& createPopTunnel(const std::string& name, const RowMeta& rowMeta){
            mfAssert(popTunnelMap.find(name) == popTunnelMap.end(),
                "duplicate find tunnel " + name);
            popTunnelMap.insert({name, new PopTunnel(rowMeta)});
            return *popTunnelMap[name];
        }

        //////// it will create automatically
        PopTunnel& getPopTunnel(const std::string name){
            mfAssert(popTunnelMap.find(name) != popTunnelMap.end(),
                "cannot find tunnel " + name);
            return *popTunnelMap.find(name)->second;
        }

    };

    struct PopTunnelable{

        PopTunnelMng* _tunnelMng = nullptr;

        explicit PopTunnelable(PopTunnelMng* tunnelMng = nullptr):
        _tunnelMng(tunnelMng){}

        [[nodiscard]]
        PopTunnel& createPopTunnel(const std::string& name, const RowMeta& rowMeta) const{
            mfAssert(_tunnelMng != nullptr, "can't create tunnel dueto miss manager");
            return _tunnelMng->createPopTunnel(name, rowMeta);
        }

        [[nodiscard]]
        PopTunnel& getPopTunnel(const std::string name) const{
            mfAssert(_tunnelMng != nullptr, "can't create tunnel dueto miss manager");
            return _tunnelMng->getPopTunnel(name);
        }

    };

}

#endif //src_lib_hw_popper_POPTUNNELMNG_H
