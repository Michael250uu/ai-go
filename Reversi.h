#ifndef Reversi_h
#define Reversi_h
#include <stdio.h>
#include "ClientSocket.h"
#include<vector>
#include <algorithm>
using namespace std;

class Reversi{
private:
    ClientSocket client_socket;
    int ownColor;
    int oppositeColor;

	//function 

	 // according to chessman position (row , col) , generate one step message in order to send to server
    void generateOneStepMessage(int row1 , int col1, int row2, int col2);

public:
	pair<pair<int, int>, pair<int, int>> step(int,int chess[19][19]);
    
    void saveChessBoard(int round);

	void handleMessage(int row1, int col1, int row2, int col2, int color,int round);
public:
    Reversi();
    ~Reversi();
	void setOwnColor(int color);

    void authorize(const char *id , const char *pass);
    
    void gameStart();
    
    void gameOver();
    
    void roundStart(int round);
    
    void oneRound(int round);
    
    void roundOver(int round);
    
    int observe(int round);
    
};



struct pos
{
    pair<int, int>step;
    bool con;//attack/protect
    int score = 0;
};

struct pos_pair
{
    pos pos1, pos2;
    int num;//attack num
    int score = 0;
};

//查找相邻位置有棋子的空位
vector<pos> find_pos(int chess[19][19],int color);

//后期测试
vector<pos> find_pos_1(int chess[19][19], int color);

//给每个位置打分
int getscore1(int chess[19][19], pair<int, int>step,int color);
int getscore0(int chess[19][19], pair<int, int> step, int color);

//方向的表示
static const int row_to[4] = { 1,1,0,-1 };
static const int col_to[4] = { 0,1,1,1 };

//分数表
static const int S[3][6] =
{
    {0,10,50,250,1250,10000},
    {0,2,10,50,250,10000},
    {0,0,0,0,0,10000}
};

//判断位置是否在棋盘内
bool check_pos(int row, int col);

#endif /* Reversi_h */
