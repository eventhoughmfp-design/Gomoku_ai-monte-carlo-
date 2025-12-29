#ifndef BITBOARD_H
#define BITBOARD_H

#include "config.h"
#include <cmath>
#include <vector>

std::vector<std::vector<Diaginfo>> init_Diag_map();

void place_piece(const ChessBoard& board,BitBoard& black,BitBoard& white,const std::vector<std::vector<Diaginfo>>& diag_map);
void place_a_piece(BitBoard& black,BitBoard& white,const std::vector<std::vector<Diaginfo>>& diag_map,int r,int c,Player player);
void erase_a_piece(BitBoard& black,BitBoard& white,const std::vector<std::vector<Diaginfo>>& diag_map,int r,int c,Player player);

inline bool has_n_in_a_row(uint16_t mask,int n) noexcept{
    uint16_t x=mask;
    for(int i=1;i<n;i++){
        x &= (mask>>i);
    }
    return x!=0;
}

#endif // BITBOARD_H
