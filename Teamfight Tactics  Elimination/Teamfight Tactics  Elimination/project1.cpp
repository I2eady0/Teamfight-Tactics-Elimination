#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <graphics.h>//easyx图形库头文件
#include <time.h>
#include "tools.h"
#include <math.h>
#include <mmsystem.h>//播放背景音乐和音效的头文件
#pragma comment(lib,"winmm.lib") //播放音乐需要的库文件
//Daily develop
//2025.3.18 构建初始界面方块
//2025.3.19 -> 2025.3.21 实现方块的移动
//2025.3.22 计算匹配次数 消除匹配 生成新方块


#define Win_Width			485//界面宽度
#define Win_Height			917//界面高度
#define Rows				8  //行数
#define Cols				8  //列数
#define Block_Type_Mount	7  //方块种类数量

	IMAGE imgBg; //背景图片
	IMAGE imgBlock[Block_Type_Mount];//小方块图片数组
//定义方块结构体

	struct block
	{
		int type;//种类 0 :none
		int x, y;
		int row, col;
		int tmp;
		int match;//匹配次数
		int transparency;//透明度 0-255 255表示完全透明，0 表示完全透明
	};
	struct block map[Rows + 2][Cols + 2];

	const int left_edge = 17;//左边界
	const int top_edge = 274;//上边界
	const int block_width = 52;//方块宽度
	const int col_width = 5;//列宽

	int Move_Flag;//移动标志位  标志当前是否正在移动 正在移动为1
	int Exchange_Flag;//当单击两个相邻方块后为1
	int Click_Flag ;//点击次数标志位 点击第二次交换
	int score;

	int pX1, pY1; //第一次单击的行和列
	int pX2, pY2; //第二次单击的行和列

void Init() //初始化
{
	//创建初始窗口
	initgraph(Win_Width, Win_Height,1);
	loadimage(&imgBg, "res/bg3.png");

	char name[64];
	for (int i = 0; i < Block_Type_Mount; i++)//打印名字数组
	{
		//res/1.png -> res/7.png
		sprintf_s(name, sizeof(name), "res/%d.png", i + 1);
		loadimage(&imgBlock[i], name, block_width, block_width,true);//绘制小方块
	}

	//配置随机数 使用时间秒数
	srand(time(NULL));

	//初始化方块数组 两次遍历
	for (int i = 0; i <= Rows; i++)
	{
		for (int j = 0; j <= Cols; j++)
		{
			map[i][j].type = 1 + rand() % 7;
			map[i][j].row = i;
			map[i][j].col = j;
			map[i][j].x = left_edge + (j - 1)* (block_width + col_width);
			map[i][j].y = top_edge + (i - 1)* (block_width + col_width);
			map[i][j].match = 0;
			map[i][j].transparency = 255;
		}
	}
	Click_Flag = 0;
	Move_Flag = 0;
	Exchange_Flag = 0;
	score = 0;
	setFont("Segoe UI Black", 20, 40);
	//播放背景音乐
	//mciSendString("play res/bg.mp3 repeat", 0, 0, 0);
	mciSendString("open res/bg.mp3 alias bgm", 0, 0, 0);
	mciSendString("play bgm repeat",0,0,0);
	mciSendString("setaudio bgm volume to 100", 0, 0, 0);//此处可以改背景音乐大小
	mciSendString("play res/start.mp3", 0, 0, 0);       //开始胖胖龙拍手声
}

void updateWindow()
{
	BeginBatchDraw();//开始双缓冲 先分配一块内存用来提前编辑
	putimage(0, 0, &imgBg);
	//循环绘出block
	for (int i = 1; i <= Rows; i++)
	{
		for (int j = 1; j <= Cols; j ++)
		{
			if (map[i][j].type != 0)
			{
				IMAGE* img = &imgBlock[map[i][j].type - 1];
				putimageTMD(map[i][j].x, map[i][j].y, img,map[i][j].transparency);
			}
		}
	}
	char scoreStr[16];
	sprintf_s(scoreStr, sizeof(scoreStr), "%d", score); //将分数的数字转为字符串
	int x = 394 + (75 - strlen(scoreStr) * 20) / 2;     //strlen(scoreStr)为字符串长度
	outtextxy(x, 60, scoreStr);                         //打印字符串


	EndBatchDraw();//结束双缓冲 释放内存
}
//block交换函数
void exchange(int row1, int col1, int row2,int col2)
{
	//此处遇到过的错误 将row放在前面，导致行列出问题
	struct block tmp = map[row1][col1];
	map[row1][col1] = map[row2][col2];
	map[row2][col2] = tmp;
	map[row1][col1].row = row1;
	map[row1][col1].col = col1;
	map[row2][col2].row = row2;
	map[row2][col2].col = col2;
	//此时保存交换后的col row到数组结构体中
}

void UserClick()//处理点击操作
{
	ExMessage msg;//识别click操作
	//如果click了 则存储到 msg中
	if (peekmessage(&msg) && msg.message == WM_LBUTTONDOWN)
	{
		/*
			map[i][j].x = left_edge + (j - 1)* (block_width + col_width);
			map[i][j].y = top_edge + (i - 1)* (block_width + col_width);
		*/
		//计算列数
		
		if (msg.x < left_edge || msg.y < top_edge) return;
		/*
		if (msg.x > left_edge + Cols * block_width + (Cols - 1) * col_width ||
			msg.y > top_edge  + Rows * block_width + (Rows - 1) * col_width) return;
		
		*/
		int col = (msg.x - left_edge) / (block_width + col_width) + 1;
		int row = (msg.y - top_edge) / (block_width + col_width) + 1;

		if (col > Cols || row > Rows) return;
		Click_Flag++;
		if (Click_Flag == 1)
		{
			pX1 = col; pY1 = row; //存入当前点击block的col row
		}
		else if (Click_Flag == 2)
		{
			pX2 = col; pY2 = row;
			if (abs(pX2 - pX1) + abs(pY2 - pY1) == 1)
			{
				exchange(pY1,pX1,pY2,pX2);
				Click_Flag = 0;
				Exchange_Flag = 1;
				//播放音效..
				PlaySound("res/clear.wav", 0, SND_FILENAME | SND_ASYNC);
			}
			else
			{
				Click_Flag = 1;//变回第一次
				pX1 = col;
				pY1 = row;
			}
		}
	}
}
void Move()//方块移动
{
	//扫描所有block
	Move_Flag = 0;
	for (int i = Rows; i > 0; i--)
	{
		for (int j = 1; j <= Cols; j++)//此处视频有误，应为 j <= Cols否则第八列无法交换
		{
			struct block* p = &map[i][j];//定义一个指针指向block位置
			int dx, dy;
			p->x;//指向此时block结构体中的x
			p->y;//指向此时block结构体中的y
				// (p->col -1) 表达结构体中的列-1 此时通过exchange函数改变的col row
				//计算出交换后的xy 储存在局部变量xy中
			for (int k = 0; k < 4; k++)
			{
				int x = left_edge + (p->col - 1) * (block_width + col_width);
				int y = top_edge + (p->row - 1) * (block_width + col_width);
				//通过dxdy即作差的算法去计算目标位置与交换前位置的差
				dx = p->x - x;
				dy = p->y - y;
				// left -> right +        right -> left -      表示方向  
				if (dx) p->x -= dx / abs(dx);   //每次 -= 后p中x的就变化 dx就变化 进而继续if
				if (dy) p->y -= dy / abs(dy);
			}
			if (dx || dy) Move_Flag = 1;

		}
	}
}
void Return()//方块还原
{

	//发生移动后，单项移动已经结束
	if (Exchange_Flag && !Move_Flag)//此处遇到的问题：count要定义在该if下 否则会持续进行exchange
	{//如果没有匹配到三个或三个以上，要还原
		int count = 0;
		for (int i = 1; i <= Rows; i++)
		{
			for (int j = 1; j <= Cols; j++)
			{
				count += map[i][j].match;
			}
		}
		if (count == 0)
		{
			exchange(pY1,pX1,pY2,pX2);
		}
		Exchange_Flag = 0;
	}
}
void Sleep() //帧等待（后续优化）
{

}
void Check()//匹配次数检查
{
	for (int i = 1 ; i <= Rows; i++)
	{
		for (int j = 1; j <= Cols; j++)
		{
			if (map[i][j].type == map[i - 1][j].type && map[i][j].type == map[i + 1][j].type)
			{
				//如果x方向三个相同，match 计数+1
				map[i][j].match++;
				map[i + 1][j].match++;
				map[i - 1][j].match++;
			}
			if (map[i][j].type == map[i][j - 1].type && map[i][j].type == map[i][j + 1].type)
			{
				//如果y方向三个相同，match 计数+1
				map[i][j].match++;
				map[i][j + 1].match++;
				map[i][j - 1].match++;
			}
		}
	}
}

void Disappear()//方块消除
{
	bool Voice_Flag = false;
	for (int i = 1; i <= Rows; i++)
	{
		for (int j = 1;j <= Cols; j++)
		{
			if (map[i][j].match && map[i][j].transparency > 10)//存在三个匹配 且透明度大于10
			{
				if (map[i][j].transparency == 255)
				{
					Voice_Flag = true;
				}
				map[i][j].transparency -= 10;
				Move_Flag = 1;
			}
		}
	}
	if (Voice_Flag)
	{
		PlaySound("res/pao.wav", 0, SND_FILENAME | SND_ASYNC);//消除声
	}
}

void Updategame()
{
	//方块下落   //写到此处发现有bug改了半天再去听视频结果他说目前就是有bug
	for (int i = Rows; i >= 1; i--)
	{
		for (int j = 1; j <= Cols; j++)
		{
			if (map[i][j].match)
			{
				for (int k = i - 1; k >= 1; k--)
				{
					if (map[k][j].match == 0)
					{
						exchange(k,j,i,j);
						break;
					}
				}
			}
		}
	}
	//生成新的方块
	for (int j = 1; j <= Cols; j++)
	{
		int n = 0;//此处不可设置初始值为1 最后的+=会持续加
		for (int i = Rows; i >= 1; i--)
		{
			if (map[i][j].match)
			{
				map[i][j].type = 1 + rand() % 7;
				map[i][j].y = top_edge - (n+1) * (block_width + col_width);
				n++;
				map[i][j].match = 0;
				map[i][j].transparency = 255;
				
			}
		}
		score += n;
	}
}

int main(void)
{
	Init();//初始化

	while (1)
	{
		UserClick();//处理点击操作
		Check(); //匹配次数检查
		Move();//方块移动
		if (!Move_Flag) Disappear();//方块消除
		Return();//方块还原
		updateWindow();//更新窗口
		if (!Move_Flag)Updategame();//更新数据(方块下落)
		if (Move_Flag) Sleep(10);//帧等待（后续优化）

	}
	system("pause");

	return 0;
}