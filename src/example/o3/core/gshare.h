//
// Created by tanawin on 13/12/25.
//

#ifndef EXAMPLE_O3_CORE_GSHARE_H
#define EXAMPLE_O3_CORE_GSHARE_H

#include "kathryn.h"
#include "mpft.h"
#include "parameter.h"


namespace kathryn::o3{

    struct Pht{ ///// pattern history table
        mMem(pht0, GSH_PHT_NUM, 2); ///// for fetch
        mMem(pht1, GSH_PHT_NUM, 2); ///// for rob

        Reg& read(opr& addr, bool first){
            ///// target resource
            MemBlock& targetBlock = first ? pht0 : pht1;
            mReg(readResult, 2);
            ///// read the register
            SET_CLK_MODE2NEG_EDGE();
            readResult <<= targetBlock[addr];
            SET_CLK_MODE2DEF();

            return readResult;
        }

        void write(opr& addr, opr& data){
            SET_CLK_MODE2NEG_EDGE();
            pht0[addr] <<= data;
            pht1[addr] <<= data;
            SET_CLK_MODE2DEF();
        }
    };


    struct Gshare{
        Mpft&   mpft;
        Pht     phts;
        RegSlot bhrs{smBhrs};

        mReg (bhrMaster, GSH_BHR_LEN);
        mWire(fetPhtVal, 2); ///// from fetch the data system
        mWire(robPhtVal, 2); ///// from rob fetch

        Gshare(Mpft& mpft): mpft(mpft){
            bhrs     .makeResetEvent(0);
            bhrMaster.makeResetEvent(0);

        }

        void buildPhtReader(opr& fetAddr,
                            opr& robAddr){
            fetPhtVal = phts.read(fetAddr, true ); ///// fetch got first bank
            robPhtVal = phts.read(robAddr, false); ///// rob got second bank
        }

        void onSucPred_bhrUpdate(opr& sucTag,
                                 opr& fetchHitAndTake){
            bhrs[OH(sucTag)] <<= gr(bhrMaster(0, GSH_BHR_LEN - 1), fetchHitAndTake);
        }

        void onMisPred_bhrUpdate(opr& misTag){

            opr& fixBhr = bhrs[OH(misTag)].v();
            for (int idx = 0; idx < SPECTAG_LEN; idx++){
                bhrs(idx) <<= fixBhr;
            }
            bhrMaster <<= fixBhr;

        }

        ///// it will invoke when btb is hit
        void onCommit_bhrUpdate(opr& fetchBtbHit,
                                opr& fetchPredCond){
            /////// bhrs
            zif (fetchBtbHit){
                for (int idx = 0; idx < SPECTAG_LEN; idx++){
                    bhrs(idx) <<= gr(bhrs(idx).sl(0, GSH_BHR_LEN - 1),  fetchPredCond);
                }
                bhrMaster <<= gr(bhrMaster(0, GSH_BHR_LEN - 1), fetchPredCond);
            }
        }

        void onCommit_PhtUpdate(opr& commitPhtAddr,
                                opr& commitCond){

            /////// calculate new value and put it back
            mVal(PHTENEG, 2, 0b00);
            mVal(PHTEPOS, 2, 0b11);
            /////// for 1 +1 /  for 0 -1  to operate the saved state machine
            opr& robPhtValOpVal = mux(commitCond, commitCond.uext(2), PHTENEG);

            opr& robPhtValNew = mux( (robPhtVal == PHTENEG) && (~commitCond), PHTENEG,
                                mux((robPhtVal == PHTEPOS) && ( commitCond), PHTEPOS,
                                robPhtVal + robPhtValOpVal
                                ));
            phts.write(commitPhtAddr, robPhtValNew);
        }

    };

}

#endif //EXAMPLE_O3_CORE_GSHARE_H