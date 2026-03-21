// //
// // Created by tanawin on 4/7/2024.
// //
// #include "kathryn.h"
// #include "gen_ele.h"
//
// namespace kathryn{
//
//     class test_gen_mod18: public Module{
//     public:
//         m_val(hf1, 8, 36);
//         m_val(hf2, 8, 12);
//         m_reg(a, 8);
//         m_reg(b, 8);
//         m_reg(c, 8);
//         m_reg(d, 8);
//         m_reg(e, 8);
//         explicit test_gen_mod18(int x): Module(){
//             a.as_output_glob("a");
//             b.as_output_glob("b");
//             c.as_output_glob("c");
//             d.as_output_glob("d");
//         }
//
//         void flow() override{
//
//             seq{
//                 par{
//                     a = 0; b = 0;
//                     c = 0; d = 0;
//                 }
//
//                 par{
//
//                     ///// fetch pipe
//                     pip("fetch"){ auto_start
//                         pip_tran("decode"){
//                             a = a + 1;
//                         }
//                     }
//                     ///// decode pipe
//                     pip("decode"){
//                         cif(a == 5){
//                             pip_tran("exec0"){
//                                 b = b + 1;
//                             }
//                         }celse{
//                             seq{
//                                 sy_wait(5);
//                                 pip_tran("exec1"){
//                                     c = c + 1;
//                                 }
//                             }
//                         }
//                     }
//
//                     ///// exec0 pipe
//                     pip("exec0"){
//                         d = d + 1;
//                     }
//                     ///// exec1 pipe
//                     pip("exec1"){
//                         e = e + 1;
//                     }
//
//                 }
//
//             }
//         }
//
//     };
//
//     class GenEle18: public GenEle{
//     public:
//         explicit GenEle18(int id): GenEle(id){};
//
//         void start(PARAM& param) override{
//             m_mod(m3, test_gen_mod18, 1);
//         }
//     };
//
//     GenEle18 test_gen18(18);
//
// }