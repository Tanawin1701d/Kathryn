//
// Created by tanawin on 4/2/2567.
//

#ifndef KATHRYN_TERMCOLOR_H
#define KATHRYN_TERMCOLOR_H

#include <iostream>

namespace kathryn{

    /** thank for source code from
     * https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
     * */
    enum Code {
        FG_RED      = 31,
        FG_GREEN    = 32,
        FG_BLUE     = 34,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
    };
    class TC {
        Code code;
    public:
        TC(Code pCode) : code(pCode) {}
        friend std::ostream&
        operator<<(std::ostream& os, const TC& mod) {
            return os << "\033[" << mod.code << "m";
        }
    };

    TC TC_BLUE(FG_BLUE);
    TC TC_RED(FG_RED);
    TC TC_GREEN(FG_GREEN);
    TC TC_DEF(FG_DEFAULT);



}

#endif //KATHRYN_TERMCOLOR_H
