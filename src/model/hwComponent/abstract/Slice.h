//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_SLICE_H
#define KATHRYN_SLICE_H

#include<cassert>

namespace kathryn{

    struct Slice {
        int start = -1; /** start index inclusive*/ /// [start, stop)
        int stop = -1;  /** stop index exclusive*/ ///{-1,-1} refer to entire section
        bool checkValidSlice() const{
            return (start >= 0) && (start < stop);
        }
        int getSize() const {return stop - start;}

        bool isBitInRangeRel(int bit){
            return (start + bit) < stop;
        }


        /** the start bit must be in range but stop but haven't to*/
        //////// b is relative value
        Slice getSubSliceWithShinkMsb(Slice b){
            assert(isBitInRangeRel(b.start));
            assert(b.checkValidSlice());
            return {start + b.start,
                    std::min(stop, start + b.stop)};
        }

        Slice getSubSlice(Slice indexer) const{
            assert(indexer.checkValidSlice());
            assert( (start + indexer.stop) <= stop);
            return {start + indexer.start, start + indexer.stop};
        }


    };

}

#endif //KATHRYN_SLICE_H
