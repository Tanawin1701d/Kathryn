//
// Created by tanawin on 21/2/2567.
//


#include "pmath.h"


namespace kathryn{

    int log2Ceil(ull val){
        return log2((val << 1) - 1);
    }

    int log2(ull val){
        return log(val, 2);
    }


    int log(ull val, int base){
        assert(val > 0);
        assert(base > 1);
        int ans = 0;

        while (val > 0){
            ans++;
            val /= base;
        }
        return ans-1;
    }

}