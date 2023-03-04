#define _CRT_SECURE_NO_WARNINGS
#include "Reversi.h"
#include<fstream>
#include<iostream>
#include<vector>
using namespace std;

#define random(x) (rand()%x)
#define ROWS 19
#define COLS 19
#define ROUNDS 2


//0是黑；1是白,需初始化全为-1。
int Chess[19][19];



Reversi::Reversi(){
    client_socket = ClientSocket();
    oppositeColor = ownColor = -1;
}

Reversi::~Reversi(){}
void Reversi::setOwnColor(int color)
{
	if (color != 0 && color != 1)
		return;
	ownColor = color;
	oppositeColor = 1 - color;

}

/*
 send id and password to server by socket
 rtn != 0 represents socket transfer error
 */
void Reversi::authorize(const char *id , const char *pass)
{
    client_socket.connectServer();
    std::cout << "Authorize " << id << std::endl;
    char msgBuf[BUFSIZE];
    memset(msgBuf , 0 , BUFSIZE);
    msgBuf[0] = 'A';
    memcpy(&msgBuf[1] , id , 9);
    memcpy(&msgBuf[10] , pass , 6);
    int rtn = client_socket.sendMsg(msgBuf);
    if (rtn != 0) printf("Authorized Failed!\n");
}

// 用户id输入，服务器上需要有对应的账号密码：对应文件 players-0.txt
void Reversi::gameStart()
{
    char id[12] = {0}, passwd[10] = {0};
	strcpy(id, ID);
	strcpy(passwd, PASSWORD);
    authorize(id, passwd);
    
    printf("Game Start!\n");
    

    for (int round = 0 ; round < ROUNDS ; round++)
	{  
		//对Chess初始化
		for (int i = 0; i <= 18; i++)
			for (int j = 0; j <= 18; j++)
				Chess[i][j] = -1;  
		Chess[9][9] = 1;

        roundStart(round);
        oneRound(round);
        roundOver(round);
    }
    gameOver();
    client_socket.close();
}

void Reversi::gameOver()
{
    printf("Game Over!\n");
}

//发一次消息，走哪一步，等两个消息，1.自己的步数行不行 2.对面走了哪一步 
void Reversi::roundStart(int round)
{
    printf("Round %d Ready Start!\n" , round);
    
    // first time receive msg from server
    int rtn = client_socket.recvMsg();
    if (rtn != 0) return ;
    if(strlen(client_socket.getRecvMsg()) < 2)
        printf("Authorize Failed!\n");
    else
        printf("Round start received msg %s\n", client_socket.getRecvMsg());
    switch (client_socket.getRecvMsg()[1]) {
            // this client : black chessman
        case 'B':
            ownColor = 0;
            oppositeColor = 1;
            rtn = client_socket.sendMsg("BB");
            printf("Send BB -> rtn: %d\n", rtn);
            if (rtn != 0) return ;
            break;
        case 'W':
            ownColor = 1;
            oppositeColor = 0;
            rtn = client_socket.sendMsg("BW");
            printf("Send BW -> rtn: %d\n", rtn);
            if (rtn != 0) return ;
            break;
        default:
            printf("Authorized Failed!\n");
            break;
    }
}

void Reversi::oneRound(int round)
{
	
    int STEP = 1;
    switch (ownColor) 
	{
        case 0:
            while (STEP < 10000) {
                
				pair<pair<int, int>, pair<int, int>> chess = step(ownColor,Chess);                        // take action, send message
                
                // lazi only excute after server's message confirm  in observe function
                generateOneStepMessage(chess.first.first, chess.first.second, chess.second.first, chess.second.second);
                
                
                if (observe(round) >= 1) break;     // receive RET Code
				

                if (observe(round) >= 1) break;    // see white move
                STEP++;
				
            }
            printf("One Round End\n");
            break;
        case 1:
            while (STEP < 10000) {
                
                if (observe(round) >= 1) break;    // see black move
				
				pair<pair<int, int>, pair<int, int>> chess = step(ownColor,Chess);                        // take action, send message
                // lazi only excute after server's message confirm  in observe function
				//cout << "?????"<<endl;
                generateOneStepMessage(chess.first.first,chess.first.second,chess.second.first, chess.second.second);
                
                
                if (observe(round) >= 1) break;     // receive RET Code
                 
                STEP++;
            }
            printf("One Round End\n");
            break;
            
        default:
            break;
    }
}

void Reversi::roundOver(int round)
{
    printf("Round %d Over!\n", round);
    // reset initializer

    ownColor = oppositeColor = -1;
}

int Reversi::observe(int round)
{
	int rtn = 0;
	int recvrtn = client_socket.recvMsg();
	if (recvrtn != 0) return 1;
	printf("receive msg %s\n", client_socket.getRecvMsg());
	switch (client_socket.getRecvMsg()[0]) 
	{
	case 'R':
	{
		switch (client_socket.getRecvMsg()[1]) 
		{
		case 'Y':   // valid step
			switch (client_socket.getRecvMsg()[2]) 
			{
			case 'P':   // update chessboard
			{
				int desRow1 = (client_socket.getRecvMsg()[3] - '0') * 10 + client_socket.getRecvMsg()[4] - '0';
				int desCol1 = (client_socket.getRecvMsg()[5] - '0') * 10 + client_socket.getRecvMsg()[6] - '0';
				int desRow2 = (client_socket.getRecvMsg()[7] - '0') * 10 + client_socket.getRecvMsg()[8] - '0';
				int desCol2 = (client_socket.getRecvMsg()[9] - '0') * 10 + client_socket.getRecvMsg()[10] - '0';
				int color = (client_socket.getRecvMsg()[11] - '0');
				//你应该在这里处理desRow和desCol，推荐使用函数
				handleMessage(desRow1, desCol1, desRow2, desCol2, color,round);

				printf("a valid step of : (%d %d) (%d %d)\n", desRow1, desCol1, desRow2, desCol2);
				break;
			}
			case 'N':   // R0N: enemy wrong step
			{
				//
				printf("a true judgement of no step\n");
				break;
			}
			}

			break;
		case 'W':
			// invalid step
			switch (client_socket.getRecvMsg()[2]) 
			{
			case 'P':
			{
				int desRow1 = (client_socket.getRecvMsg()[3] - '0') * 10 + client_socket.getRecvMsg()[4] - '0';
				int desCol1 = (client_socket.getRecvMsg()[5] - '0') * 10 + client_socket.getRecvMsg()[6] - '0';
				int desRow2 = (client_socket.getRecvMsg()[7] - '0') * 10 + client_socket.getRecvMsg()[8] - '0';
				int desCol2 = (client_socket.getRecvMsg()[9] - '0') * 10 + client_socket.getRecvMsg()[10] - '0';
				int color = (client_socket.getRecvMsg()[11] - '0');
				printf("Invalid step , server random a true step of : (%d %d) (%d %d)\n", desRow1, desCol1, desRow2, desCol2);
				//你应该在这里处理desRow和desCol，推荐使用函数
				handleMessage(desRow1, desCol1, desRow2, desCol2, color,round);
				break;
			}
			case 'N': 
			{
				printf("a wrong judgement of no step\n");
				break;
			}
			default:
				break;
			}
			break;
		case '1':

			printf("Error -1: Msg format error!\n");
			rtn = -1;
			break;
		case '2':

			printf("Error -2: Corrdinate error!\n");
			rtn = -2;
			break;
		case '4':

			printf("Error -4: Invalid step!\n");
			rtn = -4;
			break;
		default:

			printf("Error -5: Other error!\n");
			rtn = -5;
			break;
		}
		break;
	}
	case 'E':
	{
		switch (client_socket.getRecvMsg()[1]) {
		case '0':
			// game over
			rtn = 2;
			break;
		case '1':
			// round over
			rtn = 1;
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
	return rtn;
}

void Reversi::generateOneStepMessage(int row1, int col1, int row2, int col2)
{
	char msg[BUFSIZE];
	memset(msg, 0, sizeof(msg));

	//put row and col in the message
	msg[0] = 'S';
	msg[1] = 'P';
	msg[2] = '0' + row1 / 10;
	msg[3] = '0' + row1 % 10;
	msg[4] = '0' + col1 / 10;
	msg[5] = '0' + col1 % 10;
	msg[6] = '0' + row2 / 10;
	msg[7] = '0' + row2 % 10;
	msg[8] = '0' + col2 / 10;
	msg[9] = '0' + col2 % 10;
	msg[10] = '\0';

	//print
	printf("generate one step at possition (%2d,%2d,%2d,%2d) : %s\n", row1, col1, row2, col2, msg);


	client_socket.sendMsg(msg);
}

/*-------------------------last three function--------------------------------
* step : find a good position to lazi your chess.
* saveChessBoard : save the chess board now.
* handleMessage: handle the message from server.
*/

pair<pair<int, int>, pair<int, int>> Reversi::step(int owncolor,int chess[19][19])
{
	//TODO:Generate a valid step here
	/*int r1 = rand() % 19;
	int c1 = rand() % 19;
	int r2 = rand() % 19;
	int c2 = rand() % 19;*/



	

	vector<pos>pre = find_pos(chess,owncolor);

	for (vector<pos>::iterator it = pre.begin(); it < pre.end(); it++)
	{
		if (it->score >= 10000 && it->con == 1)
		{
			for(int i=0;i<=18;i++)
				for(int j=0;j<=18;j++)
					if(chess[i][j]==-1)
					{
						return make_pair(it->step, make_pair(i, j));
					}
		}
	}
	
	vector<int>v1;
	for(vector<pos>::iterator it=pre.begin();it<pre.end();it++)
	{
		int temp=it->score;
		v1.push_back(temp);
	}
	sort(v1.begin(), v1.end());
	v1.erase(unique(v1.begin(), v1.end()), v1.end());

	int max1, max2,max3;
	if (v1.size() == 1)
		max1 = max2 =max3= v1[0];
	else if(v1.size()==2)
	{
		max1 = v1[0]; max2 = max3 =v1[1];
	}
	else 
	{
		max1 = v1[v1.size() - 1]; max2 = v1[v1.size() - 2]; max3 = v1[v1.size() - 3];
	}
	
	

	//选出分数前3
	for (vector<pos>::iterator it = pre.begin(); it < pre.end();)
		if (it->score != max1 && it->score != max2&&it->score!=max3 )
			it=pre.erase(it);
		else
			it++;

	
	//分析最后一个棋子
	vector<pos_pair>last;
	for(vector<pos>::iterator it=pre.begin();it<pre.end();it++)
	{
		chess[it->step.first][it->step.second] = owncolor;
		vector<pos>lat = find_pos(chess, owncolor);
		//cout << lat[8].step.first << " " << lat[8].step.second << endl;
		for(vector<pos>::iterator ite=lat.begin();ite<lat.end();ite++)
		{
			pos_pair temp;
			temp.pos1.con = it->con; temp.pos1.score = it->score; temp.pos1.step = it->step;

			temp.pos2.con = ite->con; temp.pos2.score = ite->score; temp.pos2.step = ite->step;
			
			temp.num = it->con + ite->con;
			temp.score = it->score + ite->score;
			last.push_back(temp);
		}
		chess[it->step.first][it->step.second] = -1;
	}
	
	for (vector<pos_pair>::iterator it = last.begin(); it < last.end(); it++)
		if (it->pos2.con == 1 && it->pos2.score >= 10000)
			return make_pair(it->pos1.step, it->pos2.step);

	vector<int>sum;
	for(vector<pos_pair>::iterator it=last.begin();it<last.end();it++)
	{
		int temp = it->score;
		sum.push_back(temp);
	}
	sort(sum.begin(), sum.end());
	sum.erase(unique(sum.begin(), sum.end()), sum.end());
	
	int max4, max5, max6;
	if (sum.size() == 1)
		max4 = max5 = max6=sum[0];
	else if (sum.size() == 2)
	{
		max4 = sum[0]; max5 = max6 =sum[1];
	}
	else 
	{
		max4 = sum[sum.size() - 1]; max5 = sum[sum.size() - 2]; max6 = sum[sum.size() - 3];
	}
	
	

	//前3高分

	

	for (vector<pos_pair>::iterator it = last.begin(); it < last.end(); )
		if (it->score !=max4&&it->score!=max5&&it->score!=max6)
			it=last.erase(it);  
		else
			it++; 

	vector<pos_pair>B = last;
	//筛掉下来之后被秒杀的棋子
	for (vector<pos_pair>::iterator it = last.begin(); it < last.end(); )
	{
		chess[it->pos1.step.first][it->pos1.step.second] = chess[it->pos2.step.first][it->pos2.step.second] = owncolor;
		vector<pos>jiance = find_pos_1(chess, 1 - owncolor);
		for (vector<pos>::iterator i = jiance.begin(); i < jiance.end(); i++)
			if (i->score >= 10000)
				goto L;

		{
			vector<int>t; 
			for (vector<pos>::iterator i = jiance.begin(); i < jiance.end(); i++)
			{
				int temp = i->score; t.push_back(temp);
			}
			sort(t.begin(), t.end());
			t.erase(unique(t.begin(), t.end()), t.end());
			int m1, m2;
			if (t.size() == 1)
				m1 = m2  = t[0];
			else
			{
				m1 = t[t.size() - 1]; m2 = t[t.size() - 2];
			}
			
			for (vector<pos>::iterator i = jiance.begin(); i < jiance.end(); )
				if (i->score != m1&&i->score!=m2)
					i = jiance.erase(i);
				else
					i++;

			for (vector<pos>::iterator i = jiance.begin(); i < jiance.end(); i++)
			{
				chess[i->step.first][i->step.second] = 1 - owncolor;
				vector<pos>j2 = find_pos_1(chess, 1 - owncolor);
				for(vector<pos>::iterator i1=j2.begin();i1<j2.end();i1++)
					if(i1->score>=10000)
					{
						chess[i1->step.first][i1->step.second] = -1; goto L;
					}
					chess[i->step.first][i->step.second] = -1;
			}

		}
		chess[it->pos1.step.first][it->pos1.step.second] = chess[it->pos2.step.first][it->pos2.step.second] = -1;
		it++;
		continue;
	L:chess[it->pos1.step.first][it->pos1.step.second] = chess[it->pos2.step.first][it->pos2.step.second] = -1;
		it = last.erase(it); 
		
	}
	if (last.size() == 0)
		last = B;
	

	vector<int>tt;
	for(vector<pos_pair>::iterator it=last.begin();it<last.end();it++)
	{
		int temp = it->score;
		tt.push_back(temp);
	}
	sort(tt.begin(), tt.end());
	int M = tt[tt.size() - 1];
	
	for (vector<pos_pair>::iterator it = last.begin(); it < last.end(); )
		if (it->score != M)
			it=last.erase(it);
		else
			it++;

	vector<int>A;                                                     
	for (vector<pos_pair>::iterator it =last.begin(); it < last.end(); it++)
	{
		int temp = it->num;
		A.push_back(temp);
	}
	sort(A.begin(), A.end());
	//最xiao进攻次数
	int AM = A[0];                                             
	
	for (vector<pos_pair>::iterator it = last.begin(); it <last.end(); it++)
		if (it->num == AM)
		{
			//cout << endl<<it->pos1.step.first << "," << it->pos1.step.second << " " << it->pos2.step.first << "," << it->pos2.step.second<<endl;
			//cout << it->pos1.con << "," << it->pos1.score << " " << it->pos2.con << "," << it->pos2.score<<endl;
			//cout  << it->score << endl ;
			cout << "i am " << ownColor<<endl<<endl;
			return make_pair(it->pos1.step, it->pos2.step);
		}



	/*pair<int, int> step1 = make_pair(r1, c1);
	pair<int, int> step2 = make_pair(r2, c2);
	return make_pair(step1, step2);*/


}



void Reversi::saveChessBoard(int round)
{
	char file_name[6];
	file_name[0] = (char)(48 + round);
	file_name[1] = '.'; file_name[2] = 't'; file_name[3] = 'x'; file_name[4] = 't'; file_name[5] = '\0';
	ofstream F(file_name, ios::app);
	for (int i = 0; i <= 18; i++)
	{
		for (int j = 0; j <= 18; j++)
		{
			F << "|";
			if (Chess[i][j] == 0)
				F << "B ";
			else if (Chess[i][j] == 1)
				F << "W";
			else
				F << "__";
			F << "| ";
		}
		F << '\n';
	}
	F << endl<<endl<<endl;
	F.close();
}

void Reversi::handleMessage(int row1, int col1, int row2, int col2, int color,int round) 
{
	char file_name[6];
	file_name[0] = (char)(48 + round);
	file_name[1] = '.'; file_name[2] = 't'; file_name[3] = 'x'; file_name[4] = 't'; file_name[5] = '\0';
	ofstream F(file_name, ios::app);

	if (color)//白
	{
		F << "W turn: (" << row1 << "," << col1 << ")" << ";" << "(" << row2 << "," << col2 << ")" << '\n';
		F.close();
		Chess[row1][col1] = Chess[row2][col2] = 1;
	}
	else
	{
		F << "B turn: (" << row1 << "," << col1 << ")" << ";" << "(" << row2 << "," << col2 << ")" << '\n';
		F.close();
		Chess[row1][col1] = Chess[row2][col2] = 0;
	}
	saveChessBoard(round);

}

//查找相邻位置有棋子的空位
vector<pos> find_pos(int chess[19][19],int color)
{
	vector<pos>f;
	for(int i=0;i<=18;i++)
		for (int j = 0; j <= 18; j++)
		{
			if (chess[i][j] != -1)
				continue;
			bool flag = 0;
			if(i-1>=0&&i-1<=18&&j+1>=0&&j+1<=18)
				if (chess[i - 1][j + 1] != -1)
				 goto L;
			if(i>=0&&i<=18&&j+1>=0&&j+1<=18)
				if(chess[i][j+1]!=-1)
					goto L;
			if(i+1>=0&&i+1<=18&&j+1>=0&&j+1<=18)
				if(chess[i+1][j+1]!=-1)
					goto L;
			if(i+1>=0&&i+1<=18&&j>=0&&j<=18)
				if(chess[i+1][j]!=-1)
					goto L;
			if (i + 1 >= 0 && i + 1 <= 18 && j-1 >= 0 && j-1 <= 18)
				if (chess[i + 1][j-1] != -1)
					goto L;
			if (i  >= 0 && i <= 18 && j -1>= 0 && j-1 <= 18)
				if (chess[i ][j-1] != -1)
					goto L;
			if (i - 1 >= 0 && i - 1 <= 18 && j -1>= 0 && j-1 <= 18)
				if (chess[i - 1][j-1] != -1)
					goto L;
			if (i - 1 >= 0 && i - 1 <= 18 && j >= 0 && j <= 18)
				if (chess[i - 1][j] != -1)
					goto L;
			continue;
		L:pos temp0,temp1;//1攻击，0防御
			temp1.step =temp0.step= make_pair(i, j);
			temp1.con = 1; temp0.con = 0;
			temp1.score = getscore1(chess, make_pair(i, j), color);
			temp0.score = getscore0(chess, make_pair(i, j), 1 - color);
			f.push_back(temp0); f.push_back(temp1);
		}
	return f;
}

vector<pos> find_pos_1(int chess[19][19], int color)
{
	vector<pos>f;
	for (int i = 0; i <= 18; i++)
		for (int j = 0; j <= 18; j++)
		{
			if (chess[i][j] != -1)
				continue;
			bool flag = 0;
			if (i - 1 >= 0 && i - 1 <= 18 && j + 1 >= 0 && j + 1 <= 18)
				if (chess[i - 1][j + 1] != -1)
					goto L;
			if (i >= 0 && i <= 18 && j + 1 >= 0 && j + 1 <= 18)
				if (chess[i][j + 1] != -1)
					goto L;
			if (i + 1 >= 0 && i + 1 <= 18 && j + 1 >= 0 && j + 1 <= 18)
				if (chess[i + 1][j + 1] != -1)
					goto L;
			if (i + 1 >= 0 && i + 1 <= 18 && j >= 0 && j <= 18)
				if (chess[i + 1][j] != -1)
					goto L;
			if (i + 1 >= 0 && i + 1 <= 18 && j - 1 >= 0 && j - 1 <= 18)
				if (chess[i + 1][j - 1] != -1)
					goto L;
			if (i >= 0 && i <= 18 && j - 1 >= 0 && j - 1 <= 18)
				if (chess[i][j - 1] != -1)
					goto L;
			if (i - 1 >= 0 && i - 1 <= 18 && j - 1 >= 0 && j - 1 <= 18)
				if (chess[i - 1][j - 1] != -1)
					goto L;
			if (i - 1 >= 0 && i - 1 <= 18 && j >= 0 && j <= 18)
				if (chess[i - 1][j] != -1)
					goto L;
			continue;
		L:pos temp;
			temp.step = make_pair(i, j);
			temp.con = 1; 
			temp.score = getscore1(chess, make_pair(i, j), color);
			f.push_back(temp); 
		}
	return f;
}

//给每个位置打分
int getscore1(int chess[19][19], pair<int, int> step,int color)
{
	int score = 0;
	for (int n = 0; n <= 3; n++)//四个方向
	{
		int lianzi = 0, zudang = 0;
		for (int i = 1; i <= 5; i++)
		{
			if (check_pos(step.first + i * row_to[n], step.second + i * col_to[n]))//未出界
			{
				if (chess[step.first + i * row_to[n]][step.second + i * col_to[n]] == -1)//空
					break;
				else if (chess[step.first + i * row_to[n]][step.second + i * col_to[n]] == color)//连子
				{
					lianzi++; continue;
				}
				else//阻挡
				{
					zudang++; break;
				}
			}
			else//出界
			{
				zudang++; break;
			}
		}
		for (int i = -1; i >= -5; i--)
		{
			if (check_pos(step.first + i * row_to[n], step.second + i * col_to[n]))
			{
				if (chess[step.first + i * row_to[n]][step.second + i * col_to[n]] == -1)//空
					break;
				else if (chess[step.first + i * row_to[n]][step.second + i * col_to[n]] == color)//连子
				{
					lianzi++; continue;
				}
				else//阻挡
				{
					zudang++; break;
				}
			}
			else//出界
			{
				zudang++; break;
			}
		}
		if (lianzi > 5)lianzi = 5;
		score += S[zudang][lianzi];
	}
	return score;
}


int getscore0(int chess[19][19], pair<int, int> step, int color)
{
	int score = 0;
	for (int n = 0; n <= 3; n++)//四个方向
	{
		int lianzi = 0, zudang = 0;
		for (int i = 1; i <= 5; i++)
		{
			if (check_pos(step.first + i * row_to[n], step.second + i * col_to[n]))//未出界
			{
				if (chess[step.first + i * row_to[n]][step.second + i * col_to[n]] == -1)//空
					break;
				else if (chess[step.first + i * row_to[n]][step.second + i * col_to[n]] == color)//连子
				{
					lianzi++; continue;
				}
				else//阻挡
				{
					zudang++; break;
				}
			}
			else//出界
			{
				zudang++; break;
			}
		}
		for (int i = -1; i >= -5; i--)
		{
			if (check_pos(step.first + i * row_to[n], step.second + i * col_to[n]))
			{
				if (chess[step.first + i * row_to[n]][step.second + i * col_to[n]] == -1)//空
					break;
				else if (chess[step.first + i * row_to[n]][step.second + i * col_to[n]] == color)//连子
				{
					lianzi++; continue;
				}
				else//阻挡
				{
					zudang++; break;
				}
			}
			else//出界
			{
				zudang++; break;
			}
		}
		if (lianzi > 5)lianzi = 5;
		if(zudang==0&&lianzi==4)
		{
			score += S[zudang][lianzi] + 500;
		}
		else if(lianzi==4&&zudang==1)
		{
			score += S[zudang][lianzi] + 250;
		}
		else
			score += S[zudang][lianzi];
	}
	return score;
}

bool check_pos(int row, int col)
{
	if (row >= 0 && row <= 18 && col >= 0 && col <= 18)
		return 1;
	return 0;
}
