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

        bool isContain(const Slice rhs) const{
            return (start <= rhs.start) && (stop >= rhs.stop);
        }

        bool isIntersec(const Slice rhs)const{
            return ((start >= rhs.start) && (start < rhs.stop)) ||
                   ((rhs.start >= start) && (rhs.start < stop))
            ;
        }

        Slice getMatchSizeSubSlice(Slice b){
            assert(b.checkValidSlice());
            assert(checkValidSlice());
            return getSubSliceWithShinkMsb({0, b.getSize()});
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

        Slice getSubSlice(int startIdx) const{
            assert(startIdx < stop);
            return {start + startIdx, stop};
        }

        bool operator == (const Slice& b) const{
            return (start == b.start) && (stop == b.stop);
        }


    };

}

#endif //KATHRYN_SLICE_H
