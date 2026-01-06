#ifndef GOMOKUGAME_H
#define GOMOKUGAME_H

#include <unordered_map>
#include <utility>
#include "config.h"


//重载运算符，使ChessBoard类型可以作为unordered_map的key
bool operator==(const ChessBoard& a,const ChessBoard& b) noexcept;

class GomokuGame{

public:
    GomokuGame();

    //公共游戏接口
    void StartGame();
    ChessBoard GetCurBoard() const noexcept;
    bool Make_Move(int row,int col,Player player);   //判断当前玩家的落子是否合法
    std::pair <int,int> GetAIMove();   //获取AI落子位置
    Player CheckWinner()noexcept;
    bool is_full()noexcept; //判断局面是否满了

private:
    ChessBoard uctSearch(const ChessBoard& board,Player player,std::pair<int,int> center);                     //利用uct算法找到AI当前棋局下最优的下一步棋盘

    std::pair<ChessBoard,Player> treePolicy(ChessBoard board,Player player,std::pair<int,int> center);  //利用MCT树的逻辑，从当前盘面向下扩展，并通过比较UCB值选择一个最佳的子节点返回

    ChessBoard expand(ChessBoard board,Player player,int x1,int x2,int y1,int y2);                        //从当前棋盘向下扩展

    double default_policy(ChessBoard board,Player player);                    //对当前棋局进行推演，返回胜（1.0）负（-1.0）平（0.0）用于累加胜利次数

    double UCB(const ChessBoard& board,Player player) noexcept;                   //利用statemap找到该棋盘对应的性质并计算ucb值

    void back_up(ChessBoard current,const ChessBoard& root,double value);     //通过parentmap形成的模拟链反向传播

    void init_ChessBoard_state(const ChessBoard& board);                      //将一个新访问的棋局加入到状态列表中并对它进行初始化

    void reuse(const ChessBoard& board);                                      //节点复用，无用的节点删除

    std::pair<bool,std::pair<int,int>> check_four(const ChessBoard& board,Player player);   //检查四子相连
    std::pair<bool,std::pair<int,int>> check_three(ChessBoard board,Player player);  //检查三子相连
    std::pair<int,int> check_double_thread(const ChessBoard& board);     //检查双活三位点

    Player check_winner(const ChessBoard& board,BitBoard b_black={},BitBoard b_white={})const noexcept;                               //检查是否有获胜者
    bool check_win_on_bitboard(const BitBoard& bitboard)const noexcept;                            //用位棋盘加速

    bool is_terminal(const ChessBoard& board)const noexcept;                                  //检查棋盘是否满了

    std::pair <int,int> cal_center(const ChessBoard& board,BitBoard black={},BitBoard white={}) noexcept;                         //获取搜索中心

    int count_piece(const ChessBoard& board,int r1,int r2,int c1,int c2)const noexcept;                               //计算给定行列范围内的棋子个数

    std::vector<std::vector<Diaginfo>> diag_map;
    ChessBoard current_board;
    Player current_player;
    int round;

    std::unordered_map<ChessBoard,StateProperty,ChessBoardHash> statemap;             //将棋盘及其相关性质一一对应
    std::unordered_map<ChessBoard,ChessBoard,ChessBoardHash> parentmap;               //key为子节点，对应查找其父节点

    static constexpr int SELECT_NUM=100000;
    static constexpr int SIMULATION_NUM=1;
    int select_range;

};

#endif // GOMOKUGAME_H
