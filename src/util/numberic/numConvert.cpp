//
// Created by tanawin on 25/1/2567.
//

#include "numConvert.h"

#include <cassert>
#include <iomanip>

namespace kathryn{

    /** words = words*mul + add   (LSB-first, fixed length); returns final carry. */
    static ull wideMulAdd(std::vector<ull>& words, ull mul, ull add){
        unsigned long long carry = add;
        for (size_t i = 0; i < words.size(); i++){
            __uint128_t cur = (__uint128_t)words[i] * mul + carry;
            words[i] = (ull)cur;
            carry    = (ull)(cur >> bitSizeOfUll);
        }
        return carry;
    }

    /** parse a hex (0x..), binary (0b..) or decimal literal into LSB-first ull
     *  words sized ceil(width/64); asserts the value fits `width` bits. */
    std::vector<ull> parseLiteralToWords(const std::string& lit, int width){
        assert(width > 0);

        //// step 1: size the LSB-first word array to ceil(width / 64), zero filled
        int arrSize = (width + bitSizeOfUll - 1) / bitSizeOfUll;
        std::vector<ull> words(arrSize, 0);

        //// step 2: detect the radix from the prefix (0x -> hex, 0b -> binary, else decimal)
        ////         and skip past that prefix
        int    base  = 10;
        size_t start = 0;
        if (lit.size() >= 2 && lit[0] == '0' && (lit[1] == 'x' || lit[1] == 'X')){ base = 16; start = 2; }
        else if (lit.size() >= 2 && lit[0] == '0' && (lit[1] == 'b' || lit[1] == 'B')){ base = 2; start = 2; }

        //// step 3: fold digits left-to-right as  words = words*base + digit,
        ////         skipping grouping chars; a non-zero carry means we ran out of words
        for (size_t i = start; i < lit.size(); i++){
            char c = lit[i];
            if (c == '_' || c == ' '){ continue; }   //// allow digit grouping
            int d;
            if      (c >= '0' && c <= '9'){ d = c - '0'; }
            else if (c >= 'a' && c <= 'f'){ d = c - 'a' + 10; }
            else if (c >= 'A' && c <= 'F'){ d = c - 'A' + 10; }
            else { assert(false && "invalid digit in wide literal"); d = 0; }
            assert(d < base && "digit out of range for literal base");
            ull carry = wideMulAdd(words, (ull)base, (ull)d);
            assert(carry == 0 && "wide literal value overflows its declared width");
        }

        //// step 4: reject values that overflow a non-64-aligned width
        ////         (any bit above `width` in the top partial word must be zero)
        int topBits = width % bitSizeOfUll;
        if (topBits != 0){
            ull mask = (~0ULL) << topBits;
            assert((words[arrSize - 1] & mask) == 0 &&
                   "wide literal value overflows its declared width");
        }

        //// step 5: hand back the packed LSB-first words
        return words;
    }

    /** emit a sized Verilog literal `<width>'h<HEX>` (MSB word first). */
    std::string cvtWordsToVerilogLiteral(const std::vector<ull>& words, int width){
        assert(!words.empty());
        int hi = (int)words.size() - 1;
        while (hi > 0 && words[hi] == 0){ hi--; }   //// trim leading zero words (keep word 0)

        std::ostringstream oss;
        oss << std::hex << std::uppercase << words[hi];         //// top word: no leading zeros
        for (int i = hi - 1; i >= 0; i--){
            oss << std::setw(bitSizeOfUll / 4) << std::setfill('0') << words[i];
        }
        return std::to_string(width) + "'h" + oss.str();
    }

    ull genBiConValRep(bool val, int bitSize){
        const int MAX_BIT = 64;
        assert(bitSize <= MAX_BIT);
        if (val)
            return (bitSize < MAX_BIT) ? ((((ull)1) << bitSize) - 1) : -1;
        else
            return 0;
    }

    std::string cvtNum2HexStr(ull x){
        std::ostringstream oss;
        oss << std::hex <<std::uppercase << ull(x);
        return "0x" + oss.str();
    }

    std::string cvtNum2BinStr(ull x){
        std::bitset<64> b(x);
        std::string binStr = b.to_string();

        // Remove leading zeros
        size_t firstOne = binStr.find('1');
        if (firstOne != std::string::npos) {
            binStr = binStr.substr(firstOne);
        } else {
            binStr = "0";  // Handle the case when x is 0
        }

        return "0b" + binStr;
    }


}