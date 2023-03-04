#include "Reversi.h"
//#include"testGUI.h"
#include<fstream>
#include<time.h>
int main() 
{
	/* Normal code */
	//--------------------------------------local chessboard file
	ofstream F1("0.txt"); F1.close();
	ofstream F2("1.txt"); F2.close();
	ofstream F3("2.txt"); F3.close();
    srand((unsigned int)(time(nullptr)));
    Reversi reversi = Reversi();


    reversi.gameStart();
	system("pause");

	///* A simple GUI for test*/
	//Game test;
	////test.HumanGame(); 
	//test.AIGame(1); //humanColor = WHITE

	return 0;
}
