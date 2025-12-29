#include "BoardWidget.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent), m_gameInProgress(false), m_isHumanTurn(false){

    setMinimumSize(400,400);  // 设置一个合理的最小尺寸，防止窗口缩得太小

    updateDimensions();   // 立即计算一次绘制参数
}

void BoardWidget::startGame(){
    qDebug()<<"Starting new game...";
    m_game.StartGame(); // 逻辑核心，重置棋盘
    m_gameInProgress = true;
    m_isHumanTurn = true; // AI(黑棋)先手在天元，所以轮到玩家(白棋)
    update();   // update不会立刻调用 paintEvent，而是让Qt在下一个事件循环周期去调用，这比repaint(立刻重绘)更高效
}

void BoardWidget::updateDimensions(){
    int margin=20;   // 棋盘四周有 20 像素的“边距”

    // 棋盘是正方形，所以取窗口宽和高中较小的那个作为基准
    int boardSize=qMin(width()-2*margin, height()-2*margin);

    m_gridSize=boardSize/(BOARD_ROWS-1);   //计算每个格子的像素大小

    m_pieceRadius=static_cast<int>(m_gridSize*0.45);    // 棋子半径为格子大小的 45%

    // 棋盘的“真实”像素尺寸
    int realBoardSize=m_gridSize*(BOARD_ROWS-1);

    // 计算偏移量，实现居中
    m_offsetX=(width()-realBoardSize)/2;
    m_offsetY=(height()-realBoardSize)/2;
}

void BoardWidget::paintEvent(QPaintEvent *event){
    QWidget::paintEvent(event);

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);   // 开启“抗锯齿”，让线条和圆形更平滑

    updateDimensions();    // 每次重绘时，都重新计算一下尺寸，以支持窗口缩放

    //只在计算出的棋盘区域绘制，而不是整个控件
    QRect boardRect(m_offsetX, m_offsetY,
                    m_gridSize*(BOARD_ROWS-1),
                    m_gridSize*(BOARD_COLS-1));
    painter.fillRect(boardRect, QColor(210, 180, 140)); //棋盘颜色为棕褐色


    painter.setPen(Qt::black);

    // 绘制网格线
    for (int i=0;i<BOARD_ROWS;i++) {
        painter.drawLine(m_offsetX+i*m_gridSize, m_offsetY,
                         m_offsetX+i*m_gridSize, m_offsetY+(BOARD_COLS-1)*m_gridSize);

        painter.drawLine(m_offsetX, m_offsetY+i*m_gridSize,
                         m_offsetX+(BOARD_ROWS-1)*m_gridSize, m_offsetY+i*m_gridSize);
    }

    ChessBoard board=m_game.GetCurBoard(); // 从游戏逻辑中获取当前棋盘状态
    for (int r=0;r<BOARD_ROWS;r++) {
        for (int c=0;c<BOARD_COLS;c++) {
            Player p=board.grid[r][c];

            if (p==Player::None) {
                continue;
            }
            QPoint center(m_offsetX+c*m_gridSize, m_offsetY+r*m_gridSize);    // 计算棋子中心的像素坐标

            // 根据玩家设置颜色
            if (p==Player::Black){
                painter.setBrush(Qt::black);
            } else {
                painter.setBrush(Qt::white);
            }

            painter.drawEllipse(center, m_pieceRadius, m_pieceRadius);   // 绘制棋子
        }
    }
}

void BoardWidget::mousePressEvent(QMouseEvent *event){
    if (!m_gameInProgress||!m_isHumanTurn||event->button()!=Qt::LeftButton){
        return;
    }
    //将鼠标点击的像素坐标转换为棋盘坐标
    int x=event->pos().x()-m_offsetX;
    int y=event->pos().y()-m_offsetY;

    //计算最近的交叉点
    int row=qRound(static_cast<double>(y)/m_gridSize);
    int col=qRound(static_cast<double>(x)/m_gridSize);

    if (row<0||row>=BOARD_ROWS||col<0||col>=BOARD_COLS){
        return;
    }

    qDebug()<<"Human clicked at [row, col]:"<<row<<","<< col;

    processHumanMove(row, col);
}


void BoardWidget::processHumanMove(int row, int col){
    if (m_game.Make_Move(row, col, Player::White)) {
        m_isHumanTurn = false; // 换AI落子
        update(); // 重绘棋盘，显示玩家的棋子

        Player winner=m_game.CheckWinner();
        if (winner!=Player::None) {
            endGame(winner); // 游戏结束
            return;
        }
        QApplication::processEvents();

        processAITurn();

    } else {
        qDebug()<<"Invalid move at"<<row<<","<<col;   //无效落子
    }
}

void BoardWidget::processAITurn(){
    std::pair<int, int> aiMove = m_game.GetAIMove();
    qDebug()<<"AI moved at [row, col]:"<<aiMove.first<<","<<aiMove.second;

    m_game.Make_Move(aiMove.first, aiMove.second, Player::Black);
    update(); // 重绘棋盘，显示AI的棋子

    Player winner = m_game.CheckWinner();
    if (winner != Player::None) {
        endGame(winner);
        return;
    }

    if(m_game.is_full()){
        endGame(Player::None);
        return;
    }

    m_isHumanTurn = true;

}

void BoardWidget::endGame(Player winner)
{
    m_gameInProgress = false; // 游戏结束
    m_isHumanTurn = false;  // 谁的回合都不重要了
    if(winner==Player::Black){
        qDebug()<<"AI wins";
    }
    else if(winner==Player::White){
        qDebug()<<"Human wins";
    }
    else{
        qDebug()<<"No player win";
    }
}

