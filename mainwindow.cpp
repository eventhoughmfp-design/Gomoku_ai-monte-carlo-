#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_board=new BoardWidget(this);

    QVBoxLayout *boardLayout=new QVBoxLayout(ui->board_container);
    boardLayout->addWidget(m_board); // 把棋盘控件添加到布局中
    boardLayout->setContentsMargins(0, 0, 0, 0);   //边距设为0让棋盘填满容器

    m_board->startGame();   //启动时自动开始一局新游戏
}

MainWindow::~MainWindow()
{
    delete ui;
}
