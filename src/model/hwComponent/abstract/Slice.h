//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_SLICE_H
#define KATHRYN_SLICE_H


namespace kathryn{

    struct Slice {
        int start = -1; /** start index inclusive*/ /// [start, stop)
        int stop = -1;  /** stop index exclusive*/ ///{-1,-1} refer to entire section
        bool checkValidSlice() const{
            return ((start == -1) && (stop == -1)) || (start < stop);
        }
        bool isEntireSection() const{return (start == -1) && (stop == -1);}

        int getSize() const {return stop - start;}
    };

}

#endif //KATHRYN_SLICE_H
