//
// Created by tanawin on 22/2/2567.
//

#include "operable.h"

namespace kathryn{



    expression &Operable::operator&(const ull b) {
        return operator&(getMatchOperable(b));
    }

    expression &Operable::operator|(const ull b) {
        return operator|(getMatchOperable(b));
    }

    expression &Operable::operator^(const ull b) {
        return operator^(getMatchOperable(b));
    }

    expression &Operable::operator<<(const ull b) {
        return operator<<(getMatchOperable(b));
    }

    expression &Operable::operator>>(const ull b) {
        return operator>>(getMatchOperable(b));
    }

    expression &Operable::operator&&(const ull b) {
        return operator&&(getMatchOperable(b));
    }

    expression &Operable::operator||(const ull b) {
        return operator||(getMatchOperable(b));
    }

    expression &Operable::operator==(const ull b) {
        return operator==(getMatchOperable(b));
    }

    expression &Operable::operator!=(const ull b) {
        return operator!=(getMatchOperable(b));
    }

    expression &Operable::operator<(const ull b) {
        return operator<(getMatchOperable(b));
    }

    expression &Operable::operator<=(const ull b) {
        return operator<=(getMatchOperable(b));
    }

    expression &Operable::operator>(const ull b) {
        return operator>(getMatchOperable(b));
    }

    expression &Operable::operator>=(const ull b) {
        return operator>=(getMatchOperable(b));
    }

    expression& Operable::slt(const ull b){
        return slt(getMatchOperable(b));
    }

    expression& Operable::sgt(const ull b){
        return sgt(getMatchOperable(b));
    }

    expression &Operable::operator+(const ull b) {
        return operator+(getMatchOperable(b));
    }

    expression &Operable::operator-(const ull b) {
        return operator-(getMatchOperable(b));
    }

    expression &Operable::operator*(const ull b) {
        return operator*(getMatchOperable(b));
    }

    expression &Operable::operator/(const ull b) {
        return operator/(getMatchOperable(b));
    }

    expression &Operable::operator%(const ull b) {
        return operator%(getMatchOperable(b));
    }
    
    
    
    
}