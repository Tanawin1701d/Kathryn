#include "kathryn.h"
using namespace kathryn;

class ExampleModule: public Module{
public:
    mWire(i, 32);
    mReg(a, 32);mReg(b, 32);
    mReg(c, 32);mReg(d, 32);

    ExampleModule(int x): Module(){ i.asInputGlob(); d.asOutputGlob();}

    void flow() override{
        seq{
            a <<= i;
            par{
                cwhile(a < 8){
                    a <<= a + 1;
                    c <<= c + 1;
                }
                cwhile(b < 8){
                    b <<= b + 1;
                    d <<= d + 1;
                }
            }
            d <<= c + d;
        }
    }
};

int main(int argc, char* argv[]) {

    auto params = readParamKathryn(argv[1]);
    mMod(ex, ExampleModule, 0);
    startModelKathryn();
    startGenKathryn(params);
    resetKathryn();

}
