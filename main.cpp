#include <iostream>
#include "model/controller/controller.h"


using namespace kathryn;

int main() {

    auto x = Reg(3);

    par{
        Val y(3, "b000");
        y + (x < Val(3, "b555"));
    }

}
