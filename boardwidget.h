#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include "GomokuGame.h"

class BoardWidget : public QWidget{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);   // 防止隐式转换

signals:


    // “槽”：一种特殊的“函数”，可以“接收”来自其他控件的“信号”
public slots:
    void startGame();

protected:
    void paintEvent(QPaintEvent *event) override;    // 当Qt系统认为“这个控件该重绘了” (比如窗口缩放或被遮挡后)，就会自动调用这个函数

    void mousePressEvent(QMouseEvent *event) override;   // 当用户在这个控件上“按下鼠标”时，就会自动调用这个函数

private:
    GomokuGame m_game; // 每个棋盘控件都有一个游戏逻辑实例
    bool m_gameInProgress; // 判断游戏是否正在进行
    bool m_isHumanTurn;  // 判断当前是否轮到玩家落子，防止AI思考时玩家乱点

    // 把棋盘坐标(0-14)转换成屏幕像素坐标 (0-600)
    int m_gridSize;    // 每一格的像素大小
    int m_offsetX;     // 棋盘左上角X坐标的偏移量 (用于居中)
    int m_offsetY;     // 棋盘左上角Y坐标的偏移量 (用于居中)
    int m_pieceRadius; // 棋子的像素半径

    void updateDimensions();   // 根据当前窗口大小，重新计算上面的 m_gridSize, m_offsetX 等参数

    void processHumanMove(int row, int col);    // 处理玩家落子后的所有逻辑

    void processAITurn();     // 处理AI落子后的所有逻辑

    void endGame(Player winner);    // 游戏结束时的处理
};

#endif // BOARDWIDGET_H
