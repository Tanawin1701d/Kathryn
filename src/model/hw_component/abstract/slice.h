//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_SLICE_H
#define KATHRYN_SLICE_H

#include "cassert"
#include "algorithm"

namespace kathryn{

    struct Slice {
        int start = -1; /** start index inclusive*/ /// [start, stop)
        int stop = -1;  /** stop index exclusive*/ ///{-1,-1} refer to entire section
        bool check_valid_slice() const{
            return (start >= 0) && (start < stop);
        }
        int get_size() const {return stop - start;}

        bool is_bit_in_range_rel(int bit){
            return (start + bit) < stop;
        }

        bool is_contain(const Slice rhs) const{
            return (start <= rhs.start) && (stop >= rhs.stop);
        }

        bool is_intersec(const Slice rhs)const{
            return ((start >= rhs.start) && (start < rhs.stop)) ||
                   ((rhs.start >= start) && (rhs.start < stop))
            ;
        }

        /** b is used to retrieve size_only*/
        Slice get_match_size_sub_slice(Slice b){
            assert(b.check_valid_slice());
            assert(check_valid_slice());
            return get_sub_slice_with_shink_msb({0, b.get_size()});
        }

        /** the start bit must be in range but stop but haven't to*/
        //////// b is relative value

        Slice get_sub_slice_with_shink_msb(Slice b){
            assert(is_bit_in_range_rel(b.start));
            assert(b.check_valid_slice());
            return {start + b.start,
                    std::min(stop, start + b.stop)};

        }

        ////// indexer is relative value
        Slice get_sub_slice(Slice indexer) const{
            assert(indexer.check_valid_slice());
            assert( (start + indexer.stop) <= stop);
            return {start + indexer.start, start + indexer.stop};
        }

        Slice get_sub_slice(int start_idx) const{
            assert(start_idx < stop);
            return {start + start_idx, stop};
        }

        bool operator == (const Slice& b) const{
            return (start == b.start) && (stop == b.stop);
        }

        bool operator != (const Slice& b) const{
            return !(*this == b);
        }


    };

}

#endif //KATHRYN_SLICE_H
