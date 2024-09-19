//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_EXAMPLE_QUEUE_H
#define KATHRYN_EXAMPLE_QUEUE_H


#include "kathryn.h"

namespace kathryn{

    struct Queue{
        const int WORD_SZ = 0;
        const int WORD_AMT = 0;
        const int ADDR_WIDTH = 0;
        mMem(queueMem, WORD_AMT, WORD_SZ);
        mReg(headPos, ADDR_WIDTH);
        mReg(lastPos, ADDR_WIDTH);
        mReg(curSize, ADDR_WIDTH);

        mWire(headWord , WORD_SZ);
        mWire(deqIntend, 1);
        mWire(enIntend , 1);
        mWire(qReset   , 1);


        explicit Queue(const int word_size, const int word_amt):
        WORD_SZ(word_size),
        WORD_AMT(word_amt),
        ADDR_WIDTH(log2Ceil(word_amt)){
            assert(word_size > 0);
            assert(word_amt  > 0);
            headWord = queueMem[headPos];

            /////////// this will run every cycle
            zif (getResetSignal()){
                curSize <<= 0;
                headPos <<= 0;
                lastPos <<= 1;
            }zelif(deqIntend ^ enIntend){
                zif(deqIntend){
                    curSize <<= curSize - 1;
                }zelse{
                    curSize <<= curSize + 1;
                }
            }
            ////////////////////////////////////

        }


        Operable& isFull () { return curSize == WORD_AMT;}
        Operable& isEmpty() { return curSize == 0;}

        Wire& getFront() {return headWord;}

        ////// no execption for overflow
        void enQueue(Operable& data){
            assert(data.getOperableSlice().getSize() == WORD_SZ);
            /** it is supposed to work parallely*/
            enIntend = 1;
            queueMem[lastPos] <<= data;
            zif(lastPos == WORD_AMT){
                lastPos <<= 0;
            }zelse{
                lastPos <<= lastPos + 1;
            }

        }

        void deQueue(){
            deqIntend = 1;
            headPos <<= headPos + 1;
        }

        void reset(){
            qReset = 1;
        }


        /** get debug (this work only when simulation is started only )*/

        std::vector<std::vector<std::string>>
        getSimDebug(std::vector<int> subSizes){ ///// seperate site
            ///////// checkSum;
            int checkSum = 0;
            for (int sz: subSizes)
                checkSum += sz;
            assert(checkSum == WORD_SZ);

            ull curSizeInQueue = (ull)curSize;
            ull actualIdx      = (ull)headPos;

            ////////// retrieve the data
            std::vector<std::vector<std::string>> result;

            for (ull curRead = 0; curRead < curSizeInQueue; curRead++){
                std::vector<std::string> currentResult;
                ull readData = queueMem.at(actualIdx).getVal();
                ////////// read in each row
                for (auto rIter = subSizes.rbegin();
                          rIter != subSizes.rend();
                          rIter++){
                    ull suffixMask = (*rIter == 64) ? UINT64_MAX : ((1 << (*rIter))-1);
                    ull curSuffix = readData & suffixMask;
                    currentResult.push_back(std::to_string(curSuffix));
                    readData = readData >> (*rIter);
                }

                result.push_back(currentResult);
                actualIdx = (actualIdx + 1) % WORD_AMT;
            }
            return result;
        }



    };

}

#endif //KATHRYN_QUEUE_H
