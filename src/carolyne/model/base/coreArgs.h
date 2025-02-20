//
// Created by tanawin on 7/2/2025.
//

#ifndef SRC_CAROLYNE_MODEL_BASE_COREARGS_H
#define SRC_CAROLYNE_MODEL_BASE_COREARGS_H

#include "lib/hw/popper/popTunnel/popTunnelMng.h"
#include "lib/hw/popper/popInterrupt.h"
#include "lib/hw/popper/popMod.h"

namespace kathryn::carolyne{
    /** BLOCK NAME */
    constexpr char FETCH_BLOCK_NAME[]  = "fetchBlk";
    constexpr char DECODE_BLOCK_NAME[] = "decodeBlk";

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
    constexpr char SLOT_F_PC     [] = "pc";

    constexpr char SLOT_F_DEC_UOP[]     = "uopId";
    constexpr char SLOT_F_DEC_OPR_VALID_SRC[] = "valid_src";
    constexpr char SLOT_F_DEC_OPR_VALID_DES[] = "valid_des";
    constexpr char SLOT_F_DEC_OPR_RAW_SRC[] = "opr_raw_src";
    constexpr char SLOT_F_DEC_OPR_RAW_DES[] = "opr_raw_des";


    constexpr char SLOT_F_ALLOC_ROB_COMMIT_MODE[] = "commitMode";
    constexpr char SLOT_F_ALLOC_VALID[]           = "valid";
    constexpr char SLOT_F_ALLOC_PHYID[]           = "phyId";
    constexpr char SLOT_F_ALLOC_ARCHID[]          = "archId";
    constexpr char SLOT_F_ALLOC_DATA[]            = "data";
    constexpr char SLOT_F_ALLOC_DATA_VALID[]      = "dataValid"; //// typically size is 1

    constexpr char SLOT_F_ROB_IDX[]               = "robIdx";


    //////////////////////////////////////
    /////// tunnel signal          ///////
    //////////////////////////////////////
    ///////// Controller <-> fetch
    constexpr char TN_PC[]                = "tun_pc_req"; //// master -> |status|pc|
    /////////  fetch     <-> memport
    constexpr char TN_FETMEM[]            = "tun_ins_mem"; //// (master) -> |reqPc|  (slave) -> |rawdata|
    /////////  fetch     <-> decoder
    constexpr char TN_FETDEC[]            = "tun_fet_dec"; ////master -> |rawdata|
    /////////  decoder   <-> allc
    constexpr char TN_DECALLOC[]          = "tun_dec_alloc"; //// master -> |uopId|[]
    /////////  alloc <-> reg allc ////(master) -> |opr_raw_[src/des]|  (slave) -> |phyId|[dataValid|data]|
    constexpr char TNF_ALLOCREG_SRC[]     = "tun_alloc_regMgmt_src";
    constexpr char TNF_ALLOCREG_DES[]     = "tun_alloc_regMgmt_des";
    /////////  alloc <-> rob allc ////(master) -> |pc|archId_0|phyId_0|archId_1|phyId_1|  (slave) -> |robIdx|       ////// the arch phy id is refered to the vector of decoder
    constexpr char TN_ALLOCROB[]          = "tun_alloc_rob";




}

#endif //SRC_CAROLYNE_MODEL_BASE_COREARGS_H
