#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <graphics.h>//easyxͼ�ο�ͷ�ļ�
#include <time.h>
#include "tools.h"
#include <math.h>
#include <mmsystem.h>//���ű������ֺ���Ч��ͷ�ļ�
#pragma comment(lib,"winmm.lib") //����������Ҫ�Ŀ��ļ�
//Daily develop
//2025.3.18 ������ʼ���淽��
//2025.3.19 -> 2025.3.21 ʵ�ַ�����ƶ�
//2025.3.22 ����ƥ����� ����ƥ�� �����·���


#define Win_Width			485//������
#define Win_Height			917//����߶�
#define Rows				8  //����
#define Cols				8  //����
#define Block_Type_Mount	7  //������������

	IMAGE imgBg; //����ͼƬ
	IMAGE imgBlock[Block_Type_Mount];//С����ͼƬ����
//���巽��ṹ��

	struct block
	{
		int type;//���� 0 :none
		int x, y;
		int row, col;
		int tmp;
		int match;//ƥ�����
		int transparency;//͸���� 0-255 255��ʾ��ȫ͸����0 ��ʾ��ȫ͸��
	};
	struct block map[Rows + 2][Cols + 2];

	const int left_edge = 17;//��߽�
	const int top_edge = 274;//�ϱ߽�
	const int block_width = 52;//������
	const int col_width = 5;//�п�

	int Move_Flag;//�ƶ���־λ  ��־��ǰ�Ƿ������ƶ� �����ƶ�Ϊ1
	int Exchange_Flag;//�������������ڷ����Ϊ1
	int Click_Flag ;//���������־λ ����ڶ��ν���
	int score;

	int pX1, pY1; //��һ�ε������к���
	int pX2, pY2; //�ڶ��ε������к���

void Init() //��ʼ��
{
	//������ʼ����
	initgraph(Win_Width, Win_Height,1);
	loadimage(&imgBg, "res/bg3.png");

	char name[64];
	for (int i = 0; i < Block_Type_Mount; i++)//��ӡ��������
	{
		//res/1.png -> res/7.png
		sprintf_s(name, sizeof(name), "res/%d.png", i + 1);
		loadimage(&imgBlock[i], name, block_width, block_width,true);//����С����
	}

	//��������� ʹ��ʱ������
	srand(time(NULL));

	//��ʼ���������� ���α���
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
	//���ű�������
	//mciSendString("play res/bg.mp3 repeat", 0, 0, 0);
	mciSendString("open res/bg.mp3 alias bgm", 0, 0, 0);
	mciSendString("play bgm repeat",0,0,0);
	mciSendString("setaudio bgm volume to 100", 0, 0, 0);//�˴����Ըı������ִ�С
	mciSendString("play res/start.mp3", 0, 0, 0);       //��ʼ������������
}

void updateWindow()
{
	BeginBatchDraw();//��ʼ˫���� �ȷ���һ���ڴ�������ǰ�༭
	putimage(0, 0, &imgBg);
	//ѭ�����block
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
	sprintf_s(scoreStr, sizeof(scoreStr), "%d", score); //������������תΪ�ַ���
	int x = 394 + (75 - strlen(scoreStr) * 20) / 2;     //strlen(scoreStr)Ϊ�ַ�������
	outtextxy(x, 60, scoreStr);                         //��ӡ�ַ���


	EndBatchDraw();//����˫���� �ͷ��ڴ�
}
//block��������
void exchange(int row1, int col1, int row2,int col2)
{
	//�˴��������Ĵ��� ��row����ǰ�棬�������г�����
	struct block tmp = map[row1][col1];
	map[row1][col1] = map[row2][col2];
	map[row2][col2] = tmp;
	map[row1][col1].row = row1;
	map[row1][col1].col = col1;
	map[row2][col2].row = row2;
	map[row2][col2].col = col2;
	//��ʱ���潻�����col row������ṹ����
}

void UserClick()//����������
{
	ExMessage msg;//ʶ��click����
	//���click�� ��洢�� msg��
	if (peekmessage(&msg) && msg.message == WM_LBUTTONDOWN)
	{
		/*
			map[i][j].x = left_edge + (j - 1)* (block_width + col_width);
			map[i][j].y = top_edge + (i - 1)* (block_width + col_width);
		*/
		//��������
		
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
			pX1 = col; pY1 = row; //���뵱ǰ���block��col row
		}
		else if (Click_Flag == 2)
		{
			pX2 = col; pY2 = row;
			if (abs(pX2 - pX1) + abs(pY2 - pY1) == 1)
			{
				exchange(pY1,pX1,pY2,pX2);
				Click_Flag = 0;
				Exchange_Flag = 1;
				//������Ч..
				PlaySound("res/clear.wav", 0, SND_FILENAME | SND_ASYNC);
			}
			else
			{
				Click_Flag = 1;//��ص�һ��
				pX1 = col;
				pY1 = row;
			}
		}
	}
}
void Move()//�����ƶ�
{
	//ɨ������block
	Move_Flag = 0;
	for (int i = Rows; i > 0; i--)
	{
		for (int j = 1; j <= Cols; j++)//�˴���Ƶ����ӦΪ j <= Cols����ڰ����޷�����
		{
			struct block* p = &map[i][j];//����һ��ָ��ָ��blockλ��
			int dx, dy;
			p->x;//ָ���ʱblock�ṹ���е�x
			p->y;//ָ���ʱblock�ṹ���е�y
				// (p->col -1) ���ṹ���е���-1 ��ʱͨ��exchange�����ı��col row
				//������������xy �����ھֲ�����xy��
			for (int k = 0; k < 4; k++)
			{
				int x = left_edge + (p->col - 1) * (block_width + col_width);
				int y = top_edge + (p->row - 1) * (block_width + col_width);
				//ͨ��dxdy��������㷨ȥ����Ŀ��λ���뽻��ǰλ�õĲ�
				dx = p->x - x;
				dy = p->y - y;
				// left -> right +        right -> left -      ��ʾ����  
				if (dx) p->x -= dx / abs(dx);   //ÿ�� -= ��p��x�ľͱ仯 dx�ͱ仯 ��������if
				if (dy) p->y -= dy / abs(dy);
			}
			if (dx || dy) Move_Flag = 1;

		}
	}
}
void Return()//���黹ԭ
{

	//�����ƶ��󣬵����ƶ��Ѿ�����
	if (Exchange_Flag && !Move_Flag)//�˴����������⣺countҪ�����ڸ�if�� ������������exchange
	{//���û��ƥ�䵽�������������ϣ�Ҫ��ԭ
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
void Sleep() //֡�ȴ��������Ż���
{

}
void Check()//ƥ��������
{
	for (int i = 1 ; i <= Rows; i++)
	{
		for (int j = 1; j <= Cols; j++)
		{
			if (map[i][j].type == map[i - 1][j].type && map[i][j].type == map[i + 1][j].type)
			{
				//���x����������ͬ��match ����+1
				map[i][j].match++;
				map[i + 1][j].match++;
				map[i - 1][j].match++;
			}
			if (map[i][j].type == map[i][j - 1].type && map[i][j].type == map[i][j + 1].type)
			{
				//���y����������ͬ��match ����+1
				map[i][j].match++;
				map[i][j + 1].match++;
				map[i][j - 1].match++;
			}
		}
	}
}

void Disappear()//��������
{
	bool Voice_Flag = false;
	for (int i = 1; i <= Rows; i++)
	{
		for (int j = 1;j <= Cols; j++)
		{
			if (map[i][j].match && map[i][j].transparency > 10)//��������ƥ�� ��͸���ȴ���10
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
		PlaySound("res/pao.wav", 0, SND_FILENAME | SND_ASYNC);//������
	}
}

void Updategame()
{
	//��������   //д���˴�������bug���˰�����ȥ����Ƶ�����˵Ŀǰ������bug
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
	//�����µķ���
	for (int j = 1; j <= Cols; j++)
	{
		int n = 0;//�˴��������ó�ʼֵΪ1 ����+=�������
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
	Init();//��ʼ��

	while (1)
	{
		UserClick();//����������
		Check(); //ƥ��������
		Move();//�����ƶ�
		if (!Move_Flag) Disappear();//��������
		Return();//���黹ԭ
		updateWindow();//���´���
		if (!Move_Flag)Updategame();//��������(��������)
		if (Move_Flag) Sleep(10);//֡�ȴ��������Ż���

	}
	system("pause");

	return 0;
}