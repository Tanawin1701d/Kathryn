//
// Created by tanawin on 30_12_2024.
//

#ifndef src_carolyne_arch_base_march_abstract_TABLEMETA_H
#define src_carolyne_arch_base_march_abstract_TABLEMETA_H

#include "genRowMeta.h"

    namespace kathryn::carolyne{

        struct TableMeta{
            int     _idxSize; //// index size
            RowMeta _rowMeta;


            TableMeta(int idxSize, const RowMeta& rowMeta):
            _idxSize(idxSize), _rowMeta(rowMeta){
                crlAss(idxSize > 0, "index size of table must larger than 0");
            }

            [[nodiscard]]
            int     getAmtRow () const{return 1 << _idxSize;}
            [[nodiscard]]
            RowMeta getRowMeta() const{return _rowMeta;}

        };

    }


#endif //src_carolyne_arch_base_march_abstract_TABLEMETA_H
