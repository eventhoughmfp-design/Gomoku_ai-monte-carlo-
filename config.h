#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <vector>

constexpr int BOARD_ROWS=15;
constexpr int BOARD_COLS=15;

//定义棋盘落子状态
enum class Player:char{
    None=0,    //空位
    White=1,   //白棋（玩家）
    Black=2    //黑棋（AI）
};

struct ChessBoard{
    Player grid[BOARD_ROWS][BOARD_COLS]={};   //所有格子初始化为None
};

//用于记录节点的性质
struct StateProperty{
    double win=0.0;   //胜利次数
    double visit=0.0;    //访问次数
    std::vector <ChessBoard> children;   //用来记录每个节点推演出来的子棋局
};

struct BitBoard{
    uint16_t row[BOARD_ROWS]={0};
    uint16_t col[BOARD_COLS]={0};
    uint16_t diag1[BOARD_ROWS+BOARD_COLS-1]={0};
    uint16_t diag2[BOARD_ROWS+BOARD_COLS-1]={0};
    bool is_empty=1;
};                                                   //用位棋盘分别储存黑白子的落子情况

struct Diaginfo{               //记录对角线的下标以及相应位点下的偏移量
    int diag1_id,diag1_off;
    int diag2_id,diag2_off;
};

struct ChessBoardHash {                       //为每个棋盘计算hash值
    std::size_t operator()(const ChessBoard& board) const noexcept {       //重载（）
        std::size_t h=0;
        const std::size_t prime=1099511628211ULL;
        const std::size_t offset=1469598103934665603ULL;

        h=offset;
        for(int i=0;i<BOARD_ROWS;i++) {
            for (int j=0;j<BOARD_COLS;j++) {
                h^=static_cast<std::size_t>(board.grid[i][j]);
                h*=prime;
            }
        }
        return h;
    }
};

#endif // CONFIG_H
