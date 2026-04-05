//
// Created by tanawin on 24/12/25.
//

#ifndef EXAMPLE_O3_SIMSHARE_RECPIPSTAGE_H
#define EXAMPLE_O3_SIMSHARE_RECPIPSTAGE_H


namespace kathryn::o3{

    enum REC_PIP_STAGE{
        RPS_MPFT     = 0,
        RPS_ARF      = 1,
        RPS_RRF      = 2,
        RPS_FETCH    = 3,
        RPS_DECODE   = 4,
        RPS_DISPATCH = 5,
        RPS_RSV      = 6,
        RPS_ISSUE    = 7,
        RPS_EXECUTE  = 8,
        RPS_COMMIT   = 9,
        RPS_STBUF    = 10,
        RPS_NUM      = 11
    };

}

#endif //EXAMPLE_O3_SIMSHARE_RECPIPSTAGE_H