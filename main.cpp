#include <iostream>
#include "model/controller/controller.h"
#include "model/hwComponent/abstract/makeComponent.h"


using namespace kathryn;


class testModule:public Module{
public:

    int& ttttt = *(new int(3));
    testModule(int y, int x){}
    void flow() override{

    }

};

int main() {

    auto x = Reg(3);

    par{
        Val y(3, "b000");
        auto& test = *(new Wire(5));
    }

    int a = 10;
    int& b = a;

    Val myVal(2,"b2");
    makeWire(z, 3) = myVal;
    makeMod(ttMod, testModule,1,2);




}
