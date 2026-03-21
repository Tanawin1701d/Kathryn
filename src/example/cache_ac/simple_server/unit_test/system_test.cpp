//
// Created by tanawin on 20/9/2024.
//

#include "system_test.h"

namespace kathryn::cache_server{

    void start_simple_cache_ac_sim(PARAM& params){
        SERVER_PARAM server_param = {
                {8, OVER_WRITE, DYNAMIC_FIELD({"value1", "value0"}, {16, 16})},
                64, 1
        };

        m_mod(server, SimpleServer, server_param);
        start_model_kathryn();
        //////// start simulator
        CacheSimItf sim_itf(params, server);
        sim_itf.sim_start();
        reset_kathryn();
    }


}