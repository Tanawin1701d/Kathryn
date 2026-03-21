//
// Created by tanawin on 26/6/2024.
//

#include "gen_mng.h"

#include "util/term_color/term_color.h"
#include "params/gen_param.h"


namespace kathryn{

    std::vector<GenEle*> gen_test_case;


    void add_gen_ele(GenEle* gen_ele){
        assert(gen_ele != nullptr);
        gen_test_case.push_back(gen_ele);
    }

    bool gen_ele_cmp(GenEle*& lhs, GenEle*& rhs){
        return lhs->get_id() < rhs->get_id();
    }

    void start_gen_ele(PARAM& param){
        std::sort(gen_test_case.begin(), gen_test_case.end(), gen_ele_cmp);
        std::string origin_top_file_name = param[GEN_TOP_FILE_NAME_PARAM_IDEN];

        for (GenEle* gen_ele: gen_test_case){
            std::cout << TC_BLUE << "gen test case " << gen_ele->get_id() << TC_DEF << std::endl;
            param[GEN_TOP_FILE_NAME_PARAM_IDEN] = origin_top_file_name + std::to_string(gen_ele->get_id());
            gen_ele->start_gen(param);
            std::cout << TC_BLUE << "finish gen test case" << gen_ele->get_id() << TC_DEF << std::endl;
        }
    }




}
