// //
// // Created by tanawin on 4/7/2024.
// //
// #include "kathryn.h"
// #include "genEle.h"
//
// namespace kathryn{
//
//     class testGenMod18: public Module{
//     public:
//         mVal(hf1, 8, 36);
//         mVal(hf2, 8, 12);
//         mReg(a, 8);
//         mReg(b, 8);
//         mReg(c, 8);
//         mReg(d, 8);
//         mReg(e, 8);
//         explicit testGenMod18(int x): Module(){
//             a.asOutputGlob("a");
//             b.asOutputGlob("b");
//             c.asOutputGlob("c");
//             d.asOutputGlob("d");
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
//                     pip("fetch"){ autoStart
//                         pipTran("decode"){
//                             a = a + 1;
//                         }
//                     }
//                     ///// decode pipe
//                     pip("decode"){
//                         cif(a == 5){
//                             pipTran("exec0"){
//                                 b = b + 1;
//                             }
//                         }celse{
//                             seq{
//                                 syWait(5);
//                                 pipTran("exec1"){
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
//             mMod(m3, testGenMod18, 1);
//         }
//     };
//
//     GenEle18 testGen18(18);
//
// }