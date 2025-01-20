//
// Created by tanawin on 20/1/2025.
//

#ifndef CAROLYNE_ARCH_BASE_UTIL_METAIDENT_H
#define CAROLYNE_ARCH_BASE_UTIL_METAIDENT_H

#include<string>

namespace kathryn::carolyne{

    extern int UTM_COUNT;
    constexpr int UNSET_UTM_ID = -1;
    constexpr char UNSET_UTM_NAME[] = "UNNAMED";

    ////// this struct is used to identify type name for each UTM (UNIT TYPE META)
    struct MetaIdentifiable{

        const int UTM_ID = -1;
        std::string _utmName = UNSET_UTM_NAME;

        explicit MetaIdentifiable(const std::string& name):
        UTM_ID(UTM_COUNT),
        _utmName(name){UTM_COUNT++;}

        MetaIdentifiable():UTM_ID(UTM_COUNT){UTM_COUNT++;}

        ////// setter
        void        setUtmName(const std::string& utmName) {_utmName = utmName;}
        ///// getter
        int         getUtmId  ()  const{return UTM_ID;}
        std::string getUtmName()  const{return _utmName;}

    };

}

#endif //CAROLYNE_ARCH_BASE_UTIL_METAIDENT_H
