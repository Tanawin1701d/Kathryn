//
// Created by tanawin on 7/2/2025.
//

#ifndef SRC_CAROLYNE_MODEL_BASE_COREARGS_H
#define SRC_CAROLYNE_MODEL_BASE_COREARGS_H

#include "lib/hw/popper/popTunnel/popTunnelMng.h"
#include "lib/hw/popper/popInterrupt.h"
#include "lib/hw/popper/popMod.h"

namespace kathryn::carolyne{

    constexpr char FETCH_BLOCK_NAME[] = "fetchBlk";

    ////// it is the element that used to pass to
    ////// all block stage to give the fundamental data
    struct FunArgs{
        PopIntrMng*   intrMng   = nullptr;
        PopTunnelMng* tunnelMng = nullptr;
    };

    //////// it is used to wrap
    struct BlkPop: PopMod{
        std::string _blkName;

        BlkPop(FunArgs funArgs, std::string blkName):
            PopMod(funArgs.intrMng, funArgs.tunnelMng),
            _blkName(blkName){}

        std::string getBlkName() const{
            return _blkName;
        }

    };

    //////////////////////////////////////
    /////// interrupt signal       ///////
    //////////////////////////////////////
    constexpr char INTR_MISPRED[] = "mispredict";

    //////////////////////////////////////
    //////// base tunnel meta-data ///////
    //////////////////////////////////////
    constexpr char SLOT_F_RAWDATA[] = "rawdata";
    constexpr char SLOT_F_PC[]      = "pc";
    //////////////////////////////////////////////////////////////////////////////////
    ///////// Controller <-> fetch
    /////////////// master -> |status|pc|
    constexpr char TN_PC          []    = "tun_pc_req";
    constexpr char TN_PC_F_STATUS []    = "status";
    constexpr int  TN_PC_F_STATUS_WIDTH = 2; ////// 00 (ok go on) /// 01 (mispredict req reset) /// 10 11 (reserve)
    //////////////////////////////////////////////////////////////////////////////////
    /////////  fetch <-> memport
    constexpr char TN_FETMEM[]        = "tun_ins_mem";
    /////////////// master -> |reqPc|
    /////////////// slave -> |rawdata|
    //////////////////////////////////////////////////////////////////////////////////
    /////////  fetch <-> decoder
    constexpr char TN_FETDEC[]        = "tun_fet_dec";
    /////////////// master -> |rawdata|



}

#endif //SRC_CAROLYNE_MODEL_BASE_COREARGS_H
