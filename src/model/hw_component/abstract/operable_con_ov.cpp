//
// Created by tanawin on 22/2/2567.
//

#include "operable.h"

namespace kathryn{



    expression &Operable::operator&(const ull b) {
        return operator&(get_match_operable(b));
    }

    expression &Operable::operator|(const ull b) {
        return operator|(get_match_operable(b));
    }

    expression &Operable::operator^(const ull b) {
        return operator^(get_match_operable(b));
    }

    expression &Operable::operator<<(const ull b) {
        return operator<<(get_match_operable(b));
    }

    expression &Operable::operator>>(const ull b) {
        return operator>>(get_match_operable(b));
    }

    expression &Operable::operator&&(const ull b) {
        return operator&&(get_match_operable(b));
    }

    expression &Operable::operator||(const ull b) {
        return operator||(get_match_operable(b));
    }

    expression &Operable::operator==(const ull b) {
        return operator==(get_match_operable(b));
    }

    expression &Operable::operator!=(const ull b) {
        return operator!=(get_match_operable(b));
    }

    expression &Operable::operator<(const ull b) {
        return operator<(get_match_operable(b));
    }

    expression &Operable::operator<=(const ull b) {
        return operator<=(get_match_operable(b));
    }

    expression &Operable::operator>(const ull b) {
        return operator>(get_match_operable(b));
    }

    expression &Operable::operator>=(const ull b) {
        return operator>=(get_match_operable(b));
    }

    expression& Operable::slt(const ull b){
        return slt(get_match_operable(b));
    }

    expression& Operable::sgt(const ull b){
        return sgt(get_match_operable(b));
    }

    expression &Operable::operator+(const ull b) {
        return operator+(get_match_operable(b));
    }

    expression &Operable::operator-(const ull b) {
        return operator-(get_match_operable(b));
    }

    expression &Operable::operator*(const ull b) {
        return operator*(get_match_operable(b));
    }

    expression &Operable::operator/(const ull b) {
        return operator/(get_match_operable(b));
    }

    expression &Operable::operator%(const ull b) {
        return operator%(get_match_operable(b));
    }
    
    
    
    
}