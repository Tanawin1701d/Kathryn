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


        /**weak assign is assign that can be posible but assigner lsb must greater than a is lsb*/
        /** a is absolute value but b typically is related value*/
        /**return slice of absolute value that relate to a */
        Slice getWeakAssignSlice(Slice b){
            assert(b.start == 0);
            return {start, std::min(stop, start + b.getSize())};
        }

        Slice getSubSlice(Slice indexer) const{
            assert(indexer.start < indexer.stop);
            assert( (start + indexer.stop) <= stop);
            return {start + indexer.start, start + indexer.stop};
        }


    };

}

#endif //KATHRYN_SLICE_H
