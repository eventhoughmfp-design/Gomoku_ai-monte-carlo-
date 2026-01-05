#include "GomokuGame.h"
#include <ctime>
#include <cstdlib>
#include <unordered_set>
#include <functional>
#include <cmath>
#include <algorithm>
#include "bitBoard.h"


bool operator==(const ChessBoard& a,const ChessBoard& b) noexcept{
    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            if(a.grid[i][j]!=b.grid[i][j]){
                return false;
            }
        }
    }
    return true;
}

GomokuGame::GomokuGame(){
    srand(static_cast<unsigned int>(time(nullptr)));   //初始化随机数种子，用于后续的随机性落子
    StartGame();
}


void GomokuGame::StartGame(){
    statemap.reserve(500000);
    parentmap.reserve(500000);      //防止哈希扩容

    current_board=ChessBoard {};    //初始化棋盘
    current_board.grid[7][7]=Player::Black;  //AI黑棋先手直接落天元
    diag_map=init_Diag_map();                //初始化对角线映射
    current_player=Player::White;   //AI落完天元轮到玩家
    round=1;

    statemap.clear();
    parentmap.clear();
    //清除数据以供新游戏使用
    init_ChessBoard_state(current_board);   //添加初始棋盘
}

ChessBoard GomokuGame::GetCurBoard()const noexcept{
    return current_board;
}

bool GomokuGame::is_full()noexcept{
    return is_terminal(current_board);
}

void GomokuGame::reuse(const ChessBoard& board){
    std::unordered_set<ChessBoard,ChessBoardHash> keep;

    keep.reserve(50000);

    std::function<void(const ChessBoard&)> dfs= [&](const ChessBoard& b){     //使用lamda表达式写dfs，获取需要保留的节点
        if(keep.count(b)) return;
        keep.insert(b);
        auto it=statemap.find(b);
        if(it!=statemap.end()){
            for(const auto& child : it->second.children){
                dfs(child);
            }
        }
    };

    dfs(board);

    //开始删除
    for(auto it=statemap.begin();it!=statemap.end();){
        if(!keep.count(it->first)){
            parentmap.erase(it->first);
            it=statemap.erase(it);
        }
        else{
            it++;
        }
    }

}

bool GomokuGame::Make_Move(int row,int col,Player player){
    if(row<0||row>=BOARD_ROWS||col<0||col>=BOARD_COLS||current_board.grid[row][col]!=Player::None){
        return false;
    }
    current_board.grid[row][col]=player;    //合法位置可以落子
    round++;

    reuse(current_board);     //落完子后剪去不要的节点

    return true;
}

std::pair<int,int> GomokuGame::GetAIMove(){
    select_range=2;                  //动态更新选择范围
    if(round>20) select_range+=2;
    if(round>34) select_range+=1;
    if(round>54) select_range+=1;
    if(round>74) select_range+=1;
    std::pair<int,int> center=cal_center(current_board);
    ChessBoard bestboard=uctSearch(current_board,Player::Black,center);
    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            if(bestboard.grid[i][j]!=current_board.grid[i][j]){
                return {i,j};      //返回AI的落子位置
            }
        }
    }
    return std::make_pair(-1,-1);    //理论上不会出现
}

Player GomokuGame::CheckWinner() noexcept{
    if(check_winner(current_board)!=Player::None) return check_winner(current_board);
    return Player::None;
}

std::pair<int,int> GomokuGame::cal_center(const ChessBoard& board,BitBoard black,BitBoard white) noexcept{
    if(black.is_empty&&white.is_empty) place_piece(board,black,white,diag_map);
    int x=0,y=0,cnt=0;
    for(int i=0;i<BOARD_ROWS;i++){
        cnt+=__builtin_popcount(black.row[i])+__builtin_popcount(white.row[i]);   //获取一行的棋子数
        while(black.row[i]!=0){
            x+=i;
            y+=__builtin_ctz(black.row[i]);
            black.row[i] &= black.row[i]-1;
        }
        while(white.row[i]!=0){
            x+=i;
            y+=__builtin_ctz(white.row[i]);
            white.row[i] &= white.row[i]-1;
        }
    }
    x=std::round(1.0*x/cnt),y=std::round(1.0*y/cnt);
    return {x,y};
}

ChessBoard GomokuGame::uctSearch(const ChessBoard& board,Player player,std::pair<int,int> center){
    //启发式落子
    ChessBoard bestmove=board;

    if(round>=8){
        std::pair<bool,std::pair<int,int>> temp1=check_four(board,player);
        if(temp1.first){
            std::pair<int,int> coord=temp1.second;
            bestmove.grid[coord.first][coord.second]=player;
            return bestmove;
        }
        Player opponent=(player==Player::Black)? Player::White:Player::Black;
        std::pair<bool,std::pair<int,int>> temp2=check_four(board,opponent);
        if(temp2.first){
            std::pair<int,int> coord=temp2.second;
            bestmove.grid[coord.first][coord.second]=player;
            return bestmove;
        }
    }

    if(round>=6){
        std::pair<bool,std::pair<int,int>> temp=check_three(board,player);
        if(temp.first){
            std::pair<int,int> coord=temp.second;
            bestmove.grid[coord.first][coord.second]=player;
            return bestmove;
        }
    }

    if(round>=8){
        std::pair<int,int> coord=check_double_thread(board);
        if(coord.first!=-1){
            bestmove.grid[coord.first][coord.second]=player;
            return bestmove;
        }
    }


    if(statemap.find(board)==statemap.end()){
        init_ChessBoard_state(board);    //如果statemap里没有找到，将其添加进去
    }

    int cnt=SELECT_NUM;
    //开始进行多次选择模拟
    while(cnt--){
        std::pair<ChessBoard,Player> select_node=treePolicy(board,player,center);    //每次选择都选目前看起来最好的或最需要模拟的节点
        for(int i=0;i<SIMULATION_NUM;i++){
            double value=default_policy(select_node.first,select_node.second);
            back_up(select_node.first,board,value);           //反向传播
        }
    }

    if(!statemap[board].children.empty()){
        bestmove=statemap[board].children.front();
        for(const auto& child : statemap[board].children){
            if(statemap[bestmove].visit<=statemap[child].visit){      //最终比较探索次数以获取下一步的最佳局面
                bestmove=child;
            }
        }
    }

    return bestmove;
}

std::pair<ChessBoard,Player> GomokuGame::treePolicy(ChessBoard board,Player player,std::pair<int,int> center){
    while(check_winner(board)==Player::None&&!is_terminal(board)){
        //计算搜索范围的四角坐标
        int x1=std::max(0,center.first-select_range);
        int x2=std::min(BOARD_ROWS-1,center.first+select_range);
        int y1=std::max(0,center.second-select_range);
        int y2=std::min(BOARD_COLS-1,center.second+select_range);
        int room=(x2-x1+1)*(y2-y1+1);      //计算总共的搜索空间
        if(count_piece(board,x1,x2,y1,y2)+statemap[board].children.size()<room){
            Player p=(player==Player::Black)? Player::White:Player::Black;
            return std::make_pair(expand(board,player,center),p);     //如果当前搜索区域落子数和棋盘的子节点之和小于搜索空间，说明未扩展完
        }
        else{
            if(statemap[board].children.empty()){
                break;
            }
            ChessBoard temp=board;                          //用临时容器储存board
            double max_ucb=-1e10;
            for(const auto& child : statemap[board].children){
                double child_ucb=UCB(child,player);
                if(child_ucb>max_ucb){
                    max_ucb=child_ucb;
                    board=child;                            //比较ucb值以获取最佳模拟子节点，temp已储存board，board直接用于储存子节点
                }
            }
            parentmap[board]=temp;
            player=(player==Player::Black)? Player::White:Player::Black;
        }
        center=cal_center(board);
    }
    Player p=(player==Player::Black)? Player::White:Player::Black;       //返回子节点后要进行模拟，模拟开始时应该为对方落子，所以转换视角
    return std::make_pair(board,p);
}

ChessBoard GomokuGame::expand(ChessBoard board,Player player,std::pair<int,int> center){
    ChessBoard temp=board;                                       //用一个中间量方便后续操作
    int x1=std::max(0,center.first-select_range);
    int x2=std::min(BOARD_ROWS-1,center.first+select_range);
    int y1=std::max(0,center.second-select_range);
    int y2=std::min(BOARD_COLS-1,center.second+select_range);
    for(int i=x1;i<=x2;i++){
        for(int j=y1;j<=y2;j++){
            if(temp.grid[i][j]==Player::None){
                temp.grid[i][j]=player;
                if(statemap.find(temp)==statemap.end()){
                    init_ChessBoard_state(temp);
                    statemap[board].children.push_back(temp);
                    parentmap[temp]=board;
                    return temp;
                }
                temp.grid[i][j]=Player::None;
            }
        }
    }
    if(!statemap[board].children.empty()){
        return statemap[board].children[rand()%statemap[board].children.size()];        //前面的循环若没返回，随机返回一个子节点
    }
    return board;
}

double GomokuGame::UCB(const ChessBoard& board,Player player) noexcept{
    if(statemap[board].visit==0) return 1e9;
    const double c=1.414;
    double tol_visit=statemap[parentmap[board]].visit;         //从父结点中获取总访问次数
    double win_rate=statemap[board].win/statemap[board].visit;
    double node_visit=statemap[board].visit;
    double search_weight=(c-1.0/2.0*round/(BOARD_ROWS*BOARD_COLS))*sqrt(log(tol_visit+1.0)/(node_visit+1.0));    //加1.0是为了防止log0；
    return (player==Player::Black)? win_rate+search_weight:-win_rate+search_weight;  //取负转换视角
}

double GomokuGame::default_policy(ChessBoard board,Player player){
    int range=2;
    int pieces=count_piece(board,0,BOARD_ROWS-1,0,BOARD_COLS-1);
    if(pieces>20) range+=2;             //根据传入的节点动态改变搜索范围
    if(pieces>34) range+=1;
    if(pieces>54) range+=1;
    if(pieces>74) range+=1;
    BitBoard b_black={},b_white={};
    place_piece(board,b_black,b_white,diag_map);
    std::pair<int,int> center=cal_center(board,b_black,b_white);

    int x1=std::max(0,center.first-range);
    int x2=std::min(BOARD_ROWS-1,center.first+range);
    int y1=std::max(0,center.second-range);
    int y2=std::min(BOARD_COLS-1,center.second+range);

    std::vector<std::pair<int,int>> center_round;       //获取合法的落子位置
    std::vector<std::pair<int,int>> whole_board;

    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            if(board.grid[i][j]==Player::None){
                if(i>=x1&&i<=x2&&j>=y1&&j<=y2){
                    center_round.push_back({i,j});
                }
                whole_board.push_back({i,j});
            }
        }
    }

    while(true){
        if(check_winner(board,b_black,b_white)!=Player::None||whole_board.empty()) break;

        auto it2=whole_board.begin()+(rand()%whole_board.size());
        std::pair<int,int> coord=*it2;
        while(board.grid[coord.first][coord.second]!=Player::None){        //两个vector当中有重叠的元素，采取lazy_delete的方式
            std::swap(*it2,whole_board.back());                             //中心区域落子后不急着删除，获取全局落子坐标需要删时再删
            whole_board.pop_back();
            if(whole_board.empty()) break;
            it2=whole_board.begin()+(rand()%whole_board.size());
            coord=*it2;
        }
        if(whole_board.empty()) break;                //删除重叠坐标后，如果empty直接跳出while(true)循环

        if(!center_round.empty()){
            auto it1=center_round.begin()+(rand()%center_round.size());
            if(rand()%100<center_round.size()*5){                         //根据中心可落子的点数来设置概率，中心落子点较少时可自动退化成全局落子
                coord=*it1;
                if(board.grid[coord.first][coord.second]!=Player::None){   //如果这个点曾在全局落子时下过了，就删除这个点重新循环
                    std::swap(*it1,center_round.back());
                    center_round.pop_back();
                    continue;
                }
                std::swap(*it1,center_round.back());
                center_round.pop_back();
            }
            else{
                std::swap(*it2,whole_board.back());
                whole_board.pop_back();
            }
        }
        else{
            std::swap(*it2,whole_board.back());
            whole_board.pop_back();
        }
        board.grid[coord.first][coord.second]=player;
        place_a_piece(b_black,b_white,diag_map,coord.first,coord.second,player);
        player = (player == Player::Black ? Player::White : Player::Black);
    }

    if(check_winner(board,b_black,b_white)==Player::None) return 0.0;
    else if(check_winner(board,b_black,b_white)==Player::Black) return 1.0;
    else return -1.0;
}

void GomokuGame::back_up(ChessBoard current,const ChessBoard& root,double value){
    statemap[current].win+=value;
    statemap[current].visit++;
    while(!(current==root)){
        if(parentmap.find(current)==parentmap.end()) break;
        current=parentmap[current];
        statemap[current].win+=value;
        statemap[current].visit++;
    }
}

void GomokuGame:: init_ChessBoard_state(const ChessBoard& board){
    StateProperty p;
    p.visit=0.0;
    p.win=0.0;
    statemap[board]=p;
}

bool GomokuGame::check_win_on_bitboard(const BitBoard& bitboard)const noexcept{         //五子相连算胜，六子相连不算
    for(int r=0;r<BOARD_ROWS;r++){
        if(has_n_in_a_row(bitboard.row[r],5)&&!has_n_in_a_row(bitboard.row[r],6)) return true;
    }
    for(int c=0;c<BOARD_COLS;c++){
        if(has_n_in_a_row(bitboard.col[c],5)&&!has_n_in_a_row(bitboard.col[c],6)) return true;
    }
    for(int d1=0;d1<BOARD_ROWS+BOARD_COLS-1;d1++){
        if(has_n_in_a_row(bitboard.diag1[d1],5)&&!has_n_in_a_row(bitboard.diag1[d1],6)) return true;
    }
    for(int d2=0;d2<BOARD_ROWS+BOARD_COLS-1;d2++){
        if(has_n_in_a_row(bitboard.diag2[d2],5)&&!has_n_in_a_row(bitboard.diag2[d2],6)) return true;
    }
    return false;
}

bool GomokuGame::is_terminal(const ChessBoard& board)const noexcept{
    bool flag=true;
    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            if(board.grid[i][j]==Player::None){
                flag=false;
                return flag;
            }
        }
    }
    return flag;
}

Player GomokuGame::check_winner(const ChessBoard& board,BitBoard b_black,BitBoard b_white)const noexcept{
    if(b_black.is_empty&&b_white.is_empty){
        place_piece(board,b_black,b_white,diag_map);
    }
    if(check_win_on_bitboard(b_black)) return Player::Black;
    if(check_win_on_bitboard(b_white)) return Player::White;
    return Player::None;
}

int GomokuGame::count_piece(const ChessBoard& board,int r1,int r2,int c1,int c2)const noexcept{
    int cnt=0;
    for(int i=r1;i<=r2;i++){
        for(int j=c1;j<=c2;j++){
            if(board.grid[i][j]!=Player::None){
                cnt++;
            }
        }
    }
    return cnt;
}

std::pair<bool,std::pair<int,int>> GomokuGame::check_four(const ChessBoard& board,Player player){
    BitBoard b_black,b_white;
    place_piece(board,b_black,b_white,diag_map);
    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            if(board.grid[i][j]==Player::None){
                place_a_piece(b_black,b_white,diag_map,i,j,player);
                if(check_winner(board,b_black,b_white)==player){
                    return {true,{i,j}};
                }
                erase_a_piece(b_black,b_white,diag_map,i,j,player);
            }
        }
    }
    return {false,{0,0}};
}

std::pair<bool,std::pair<int,int>> GomokuGame::check_three(ChessBoard board,Player player){    //自己先落一子，对手再落一子，check_four
    Player opponent=(player==Player::Black)? Player::White:Player::Black;
    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            if(board.grid[i][j]!=Player::None) continue;
            board.grid[i][j]=player;
            bool flag=true;
            for(int k1=std::max(0,i-4);k1<=std::min(BOARD_ROWS-1,i+4);k1++){             //缩小搜索范围，只在落子点的周围下子
                for(int k2=std::max(0,j-4);k2<=std::min(BOARD_COLS-1,j+4);k2++){
                    if(board.grid[k1][k2]!=Player::None) continue;
                    board.grid[k1][k2]=opponent;
                    if(check_four(board,player).first==false) flag=false;
                    board.grid[k1][k2]=Player::None;
                }
            }
            if(flag==true){
                return {true,{i,j}};
            }
            board.grid[i][j]=Player::None;
        }
    }
    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            if(board.grid[i][j]!=Player::None) continue;
            board.grid[i][j]=opponent;
            bool flag=true;
            for(int k1=std::max(0,i-4);k1<=std::min(BOARD_ROWS-1,i+4);k1++){
                for(int k2=std::max(0,j-4);k2<=std::min(BOARD_COLS-1,j+4);k2++){
                    if(board.grid[k1][k2]!=Player::None) continue;
                    board.grid[k1][k2]=player;
                    if(check_four(board,opponent).first==false) flag=false;
                    board.grid[k1][k2]=Player::None;
                }
            }
            if(flag==true){
                return {true,{i,j}};
            }
            board.grid[i][j]=Player::None;
        }
    }
    return {false,{0,0}};     //未找到
}

void threads(Player& p1,Player& p2,Player& p3,Player& p4,double& b_threads,double& w_threads){
    if((p1==p2)&&(p3==p4)&&(p3==Player::None)){
        if(p1==Player::Black) b_threads++;
        else if(p1==Player::White) w_threads++;
    }   //OXXOO

    if((p1==p2)&&(p2==p3)&&(p4==Player::None)){
        if(p1==Player::Black) b_threads++;
        else if(p1==Player::White) w_threads++;
    }   //XXXOO

    if((p1==p4)&&(p2==p3)&&(p1==Player::None)){
        if(p2==Player::Black) b_threads++;
        else if(p2==Player::White) w_threads++;
    }   //XXOOO

    if((p1==p2)&&(p2==p4)&&p3==Player::None){
        if(p1==Player::Black) b_threads++;
        else if(p1==Player::White) w_threads++;
    }   //OXXOX
    if((p1==p3)&&(p3==p4)&&(p2==Player::None)){
        if(p1==Player::Black) b_threads++;
        else if(p1==Player::White) w_threads++;
    }   //XOXOX
    if((p1==p4)&&(p2==Player::None)){
        if(p1==Player::Black) b_threads+=0.5;
        else if(p1==Player::White) w_threads+=0.5;
    }   //?OXOX  由于对称，另一侧还会再算一遍，所以 +0.5
}

std::pair<int,int> GomokuGame::check_double_thread(const ChessBoard& board){
    double b_threads=0,w_threads=0;
    for(int i=0;i<BOARD_ROWS;i++){
        for(int j=0;j<BOARD_COLS;j++){
            if(board.grid[i][j]!=Player::None) continue;
            if(j-3>=0&&j+2<BOARD_COLS){
                Player p1=board.grid[i][j-1],p2=board.grid[i][j-2],p3=board.grid[i][j-3],p4=board.grid[i][j+1];  //向左延伸
                threads(p1,p2,p3,p4,b_threads,w_threads);
                if(i-3>=0&&i+2<BOARD_ROWS){
                    p1=board.grid[i-1][j-1],p2=board.grid[i-2][j-2],p3=board.grid[i-3][j-3],p4=board.grid[i+1][j+1];  //主对角延伸
                    threads(p1,p2,p3,p4,b_threads,w_threads);
                }
                if(i+3<BOARD_ROWS&&i-2>=0){
                    p1=board.grid[i+1][j-1],p2=board.grid[i+2][j-2],p3=board.grid[i+3][j-3],p4=board.grid[i-1][j+1];  //副对角延伸
                    threads(p1,p2,p3,p4,b_threads,w_threads);
                }
            }
            if(j+3<BOARD_COLS&&j-2>=0){
                Player p1=board.grid[i][j+1],p2=board.grid[i][j+2],p3=board.grid[i][j+3],p4=board.grid[i][j-1];  //向右延伸
                threads(p1,p2,p3,p4,b_threads,w_threads);
                if(i-3>=0&&i+2<BOARD_ROWS){
                    p1=board.grid[i-1][j+1],p2=board.grid[i-2][j+2],p3=board.grid[i-3][j+3],p4=board.grid[i+1][j-1];  //副对角延伸
                    threads(p1,p2,p3,p4,b_threads,w_threads);
                }
                if(i+3<BOARD_ROWS&&i-2>=0){
                    p1=board.grid[i+1][j+1],p2=board.grid[i+2][j+2],p3=board.grid[i+3][j+3],p4=board.grid[i-1][j-1];  //主对角延伸
                    threads(p1,p2,p3,p4,b_threads,w_threads);
                }
            }
            if(i-3>=0&&i+2<BOARD_ROWS){
                Player p1=board.grid[i-1][j],p2=board.grid[i-2][j],p3=board.grid[i-3][j],p4=board.grid[i+1][j];   //向上延伸
                threads(p1,p2,p3,p4,b_threads,w_threads);
            }
            if(i+3<BOARD_ROWS&&i-2>=0){
                Player p1=board.grid[i+1][j],p2=board.grid[i+2][j],p3=board.grid[i+3][j],p4=board.grid[i-1][j];   //向下延伸
                threads(p1,p2,p3,p4,b_threads,w_threads);
            }
            if(std::round(b_threads)>=2||std::round(w_threads)>=2) return {i,j};
            b_threads=0,w_threads=0;
        }
    }
    return {-1,-1};
}
