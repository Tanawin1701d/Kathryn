//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "gen_ele.h"

namespace kathryn{

    class test_gen_mod22: public Module{
    public:
        m_val(hf1, 8, 36);
        m_val(hf2, 8, 12);
        m_reg(a, 8);
        m_wire(b, 8);
        m_reg(c, 8);
        m_wire(d, 1);
        explicit test_gen_mod22(int x): Module(){
            a.as_output_glob("a");
            b.as_output_glob("b");
            c.as_output_glob("c");
            d.as_output_glob("d");
        }

        void flow() override{

            ////////// we use d as a interrupt
            seq{
                par{
                    a = 0;
                    b = 0;
                    c = 0;
                    d = 0;
                }

                // pip_wrap{
                //     /////// pipe block 0
                //     pip_blk{
                //         intr_start(d);
                //         intr_reset(d);
                //
                //         cif(a < 5){
                //             a = a + 1;
                //         }celse{
                //             cwhile(true){
                //                 seq {
                //                     b  = 1;
                //                 }
                //             }
                //         }
                //     }
                //     /////// pipe block 1
                //     pip_blk{
                //         c = c + 1;
                //     }
                // }

            }

            seq{
                sy_wait(15);
                par{
                    a = 0;
                    d = 1;
                }
            }


        }

    };

    class GenEle22: public GenEle{
    public:
        explicit GenEle22(int id): GenEle(id){};

        void start(PARAM& param) override{
            m_mod(m3, test_gen_mod22, 1);
        }
    };

    GenEle22 test_gen22(22);

}