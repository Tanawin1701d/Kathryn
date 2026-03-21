//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_EXAMPLE_QUEUE_H
#define KATHRYN_EXAMPLE_QUEUE_H


#include "kathryn.h"
#include "../field/dynamic_field.h"

namespace kathryn{

    struct q_meta{
        std::string name;
        int         size;

    };

    struct Queue{
        DYNAMIC_FIELD _fields;
        const int WORD_SZ = 0;
        const int WORD_AMT = 0;
        const int ADDR_WIDTH = 0;
        m_mem(queue_mem, WORD_AMT, WORD_SZ);
        m_reg(head_pos , ADDR_WIDTH);
        m_reg(last_pos , ADDR_WIDTH);
        m_reg(cur_size , ADDR_WIDTH);

        m_wire(head_word , WORD_SZ);
        m_wire(deq_intend, 1);
        m_wire(en_intend , 1);


        explicit Queue(int word_amt, DYNAMIC_FIELD fields):
        _fields(fields),
        WORD_SZ(_fields.sum_field_size()),
        WORD_AMT(word_amt),
        ADDR_WIDTH(log2Ceil(word_amt)){
            assert(WORD_SZ > 0);
            assert(word_amt  > 0);
        }



        Operable& is_full () { return cur_size == WORD_AMT;}
        Operable& is_empty() { return cur_size == 0;}

        Wire& get_front() {return head_word;}

        Operable& get_front(const std::string& name){
            int idx = _fields.find_idx(name);
            int start_bit = _fields.find_start_bit(idx);
            int stop_bit  = start_bit + _fields.get_size(idx);
            return get_front()(start_bit, stop_bit);
        }

        std::vector<Operable*> get_front(const std::vector<std::string>& names){
            std::vector<Operable*> result;
            for (std::string name: names){
                result.push_back(&get_front(name));
            }
            return result;
        }

        ////// no execption for overflow
        void en_queue(Operable& data){
            assert(data.get_operable_slice().get_size() == WORD_SZ);
            /** it is supposed to work parallely*/
            en_intend = 1;
            queue_mem[last_pos] <<= data;
            zif(last_pos == (WORD_AMT-1)){
                last_pos <<= 0;
            }zelse{
                last_pos <<= last_pos + 1;
            }
        }

        void de_queue(){
            deq_intend = 1;
            zif (head_pos == (WORD_AMT-1)){
                head_pos <<= 0;
            }zelse{
                head_pos <<= head_pos + 1;
            }
        }

        void init_logic(){
            head_word = queue_mem[head_pos];
            /////////// this will run every cycle
            zif (get_reset_signal()){
                cur_size <<= 0;
                head_pos <<= 0;
                last_pos <<= 0;
            }zelif(deq_intend ^ en_intend){
                zif(deq_intend){
                    cur_size <<= cur_size - 1;
                }zelse{
                    cur_size <<= cur_size + 1;
                }
            }
        }

        /** get debug (this work only when simulation is started only )*/

        std::vector<std::vector<std::string>>
        get_sim_debug(std::vector<int> sub_sizes){ ///// seperate site
            ///////// sub_size[n] | sub_size[n-1]| sub_size[n-2] | ..... | sub_size[0] |
            ///////// check_sum;
            int check_sum = 0;
            for (int sz: sub_sizes)
                check_sum += sz;
            assert(check_sum == WORD_SZ);

            ull cur_size_in_queue = (ull)cur_size;
            ull actual_idx      = (ull)head_pos;

            ////////// retrieve the data
            std::vector<std::vector<std::string>> result;

            for (ull cur_read = 0; cur_read < cur_size_in_queue; cur_read++){
                std::vector<std::string> current_result;
                ull read_data = queue_mem.at(actual_idx).get_val();
                ////////// read in each row
                for (auto r_iter = sub_sizes.rbegin();
                          r_iter != sub_sizes.rend();
                          r_iter++){
                    ull suffix_mask = (*r_iter == 64) ? UINT64_MAX : ((((ull)1) << (*r_iter))-1);
                    ull cur_suffix = read_data & suffix_mask;
                    current_result.push_back(std::to_string(cur_suffix));
                    read_data = read_data >> (*r_iter);
                }

                result.push_back(current_result);
                actual_idx = (actual_idx + 1) % WORD_AMT;
            }
            return result;
        }

        bool check_valid_value(ull value) const{
            ull check_value = WORD_SZ == 64 ? UINT64_MAX : ((((ull) 1) << WORD_SZ) - 1);
            check_value = ~check_value;

            return !(value & check_value);
        }

        void push_data_sim(ull value){
            assert(check_valid_value(value));
            assert( ((ull)cur_size) != WORD_AMT);
            ////////// std::cout << (ull)cur_size << std::endl;
            ValRepBase cs   = ((ValRepBase)cur_size);
            ValRepBase lp   = ((ValRepBase)last_pos);

            ////////// set new cur_size
            cs.set_var(cs.get_val() + 1);
            ////////// modify the data
            queue_mem.at((ull)lp).set_var(value);
            /////////  update last pos
            std::cout << "lp bef-> " << lp.get_val() << std::endl;
            lp.set_var(( ((ull)lp) == (WORD_AMT-1) ) ? 0: ((ull)lp + 1));
            std::cout << "lp after-> " << lp.get_val() << std::endl;

        }
    };

}

#endif //KATHRYN_QUEUE_H
