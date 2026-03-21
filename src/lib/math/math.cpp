//
// Created by tanawin on 6/2/26.
//

#include "math.h"

namespace kathryn{

    Operable& sqrt_int(Operable& x){
        int bs = x.get_operable_slice().get_size();
        assert(bs > 0);
        m_reg(xc, bs);
        m_reg(y, bs);
        seq{
            par{ xc <<= x; y<<= x;}
            Operable& y_next = (y + xc/y) >> 1;
            cdowhile(y_next < y){
                y <<= y_next;
            }
        }
        return y;
    }

    void sqrt_int(Operable& x, Reg& y){
        int bs = x.get_operable_slice().get_size();
        assert(bs > 0);
        assert(bs == y.get_operable_slice().get_size());
        m_reg(xc, bs);
        seq{
            par{ xc <<= x; y<<= x;}
            Operable& y_next = (y + xc/y) >> 1;
            cdowhile(y_next < y){
                y <<= y_next;
            }
        }
    }
}