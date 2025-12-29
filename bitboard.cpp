#include "bitBoard.h"

std::vector<std::vector<Diaginfo>> init_Diag_map(){
    std::vector<std::vector<Diaginfo>> diag_map(BOARD_ROWS,std::vector<Diaginfo>(BOARD_COLS));
    for(int r=0;r<BOARD_ROWS;r++){
        for(int c=0;c<BOARD_COLS;c++){
            diag_map[r][c].diag1_id=r-c+(BOARD_COLS-1);
            diag_map[r][c].diag1_off=std::min(r,c);

            diag_map[r][c].diag2_id=r+c;
            diag_map[r][c].diag2_off=std::min(r,BOARD_COLS-1-c);
        }
    }
    return diag_map;
}

void place_piece(const ChessBoard& board,BitBoard& black,BitBoard& white,const std::vector<std::vector<Diaginfo>>& diag_map){
    for(int r=0;r<BOARD_ROWS;r++){
        for(int c=0;c<BOARD_COLS;c++){
            if(board.grid[r][c]==Player::None) continue;
            if(board.grid[r][c]==Player::Black){
                place_a_piece(black,white,diag_map,r,c,Player::Black);
            }
            else{
                place_a_piece(black,white,diag_map,r,c,Player::White);
            }
        }
    }
}

void place_a_piece(BitBoard& black,BitBoard& white,const std::vector<std::vector<Diaginfo>>& diag_map,int r,int c,Player player){
    if(player==Player::Black){
        black.row[r] |= (1u<<c);
        black.col[c] |= (1u<<r);
        black.diag1[diag_map[r][c].diag1_id] |= (1u<<diag_map[r][c].diag1_off);
        black.diag2[diag_map[r][c].diag2_id] |= (1u<<diag_map[r][c].diag2_off);
        black.is_empty=0;
    }
    else{
        white.row[r] |= (1u<<c);
        white.col[c] |= (1u<<r);
        white.diag1[diag_map[r][c].diag1_id] |= (1u<<diag_map[r][c].diag1_off);
        white.diag2[diag_map[r][c].diag2_id] |= (1u<<diag_map[r][c].diag2_off);
        white.is_empty=0;
    }
}

void erase_a_piece(BitBoard& black,BitBoard& white,const std::vector<std::vector<Diaginfo>>& diag_map,int r,int c,Player player){
    if(player==Player::Black){
        black.row[r] &= ~(1u<<c);
        black.col[c] &= ~(1u<<r);
        black.diag1[diag_map[r][c].diag1_id] &= ~(1u<<diag_map[r][c].diag1_off);
        black.diag2[diag_map[r][c].diag2_id] &= ~(1u<<diag_map[r][c].diag2_off);
    }
    else{
        white.row[r] &= ~(1u<<c);
        white.col[c] &= ~(1u<<r);
        white.diag1[diag_map[r][c].diag1_id] &= ~(1u<<diag_map[r][c].diag1_off);
        white.diag2[diag_map[r][c].diag2_id] &= ~(1u<<diag_map[r][c].diag2_off);
    }
}


