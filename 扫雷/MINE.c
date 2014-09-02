/*----------------------------------------------
   扫雷-- Version using Common Dialog Box
                (c) 舒茂军, 2014
  ----------------------------------------------*/
#include<Windows.h>
#include<time.h>
#include<stdio.h>
#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")
#include"resource.h"
#define ID_TIMER 1
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);//窗口消息处理函数
void	  Draw_3D(HWND ,HDC ,RECT);//绘制具有立体感边框函数
int		  WndWidth(int ,int ,int );				//计算窗口宽度函数
int		  WndHeight(int,int,int);					//计算窗口高度函数
void	   DrawBrick(HWND hwnd, HDC hdc,RECT rect,  int brickIndex, HBITMAP hBitMap[],int bmpIndex);//绘制方块和笑脸函数 
void    Mines_Initialization(int,int,int  );		 //初始化整个雷区函数
int		   Judge_AroundMines(int ,int,int,int);//判断一个块周围的雷数函数
void    Draw_Counts(HWND hwnd,HDC hdc,RECT rect,int Mine,HBITMAP hBitMap[],int index);//绘制计雷器和计时器函数
RECT   GetBricks_i_j_Rect(int i, int j,RECT rect);//根据i,j获得Bricks[i][j]的矩形区域
int	 Mine_Clear(int i,int j,HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int bmpIndex);//当点击到空白时显示其周围最大可显示的区域
void Double_Click(int i,int j,HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int bmpIndex);//鼠标双击时某个已显示方块时的效果
int GameWin();//判断游戏是否取得胜利
BOOL CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);//About对话框 的消息函数
BOOL CALLBACK UserProc(HWND, UINT, WPARAM,LPARAM );//用户自定义消息函数
BOOL CALLBACK HeroProc(HWND, UINT, WPARAM,LPARAM );//游戏胜利弹出填写玩家姓名的消息函数
BOOL CALLBACK HeroListProc(HWND, UINT, WPARAM,LPARAM );//扫雷英雄榜消息处理函数

//全局变量的定义
static  TCHAR szAppName[]=TEXT("扫雷--舒茂军 学号:2013E8009061062");
static int xBricks,yBricks,Mines;					//横坐标和纵坐标上方块的个数,Mine表示地雷数
static int Bricks[30][24]={0};//记录雷区的雷的布局以及雷周围的数字
static int Map[30][24]={0};//标记相应方块的状态(0、1、2、3、4、5)，若是空白则为0；若被左击过Map[i][j]=1;若右击插旗Map[i][j]=2;若左击踩到雷(第一颗雷)则Map[i][j]=3;若右击显示问号则Map[i][j]=4;若因为插旗错误而踩到雷则为5
static TCHAR WinName[20];//玩家破纪录后输入的名字
static int iLevelSelect = IDM_PRIMARY;				//用来标识菜单中的游戏级别的选择
static int timeCount;														//计时器
static int game_State;												//标识游戏未开始0、开始1、结束(获胜结束2，踩到雷结束3)三个状态
static RECT Mine_Rect; //雷区矩形区域
static int Sound = 0;
int WINAPI	WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR szCmdLine, int iCmdShow)
{
	HWND		hWnd;
	WNDCLASS	wndClass;
	MSG			msg;
	HICON hIcon;
	wndClass.style			=	CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc	=	WndProc;
	wndClass.cbClsExtra		=	0;
	wndClass.cbWndExtra		=	0;
	wndClass.hIcon			=	LoadIcon(NULL,MAKEINTRESOURCE(IDI_APPICON));
	wndClass.hInstance		=	hInstance;
	wndClass.hCursor		=	LoadCursor(NULL,IDC_ARROW);
	wndClass.hbrBackground	=	(HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndClass.lpszMenuName	=	MAKEINTRESOURCE(IDM_MINE);
	wndClass.lpszClassName	=	szAppName;
	if(!RegisterClass(&wndClass))
	{
		MessageBox(NULL,TEXT("This Program Requires Windows NT!"),szAppName,MB_ICONERROR);
		return 0;
	}

	hWnd	=	CreateWindow(szAppName,
							 szAppName,
							 WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,//不可最大化
							 CW_USEDEFAULT,
							 CW_USEDEFAULT,
							 CW_USEDEFAULT,
							 CW_USEDEFAULT,
							 NULL,
							 NULL,
							 hInstance,
							 NULL);
	 hIcon = LoadIcon((HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE) ,MAKEINTRESOURCE(IDI_APPICON));
	 SendMessage(hWnd, WM_SETICON, TRUE,  (LPARAM)hIcon);
	 ShowWindow(hWnd,iCmdShow);
	 UpdateWindow(hWnd);
	
	 while(GetMessage(&msg,NULL,0,0))
	 {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	 }
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	HDC hdcDes,hdcSour;
	static BITMAP bitMap ;
	static HBITMAP hBitMap[6] ;							//位图句柄数组
	static int cxClient,cyClient,cxSource,cySource;		//cxClient,cyClient客户区长度和宽度，cxSource，cySource资源文件的位置	
	static int wndBorderWidth,wndBorderHeight;			//窗口的长度和宽度
	static int cxBorder,cyBorder,cyCaption,cyMenu;      //表示窗口的外围边框的参数，横向边框的一个宽度，底部边框的高度，标题栏的高度，菜单栏的高度                   
	static int Bricks_C_or_B,Counts_C_or_B,Smiles_C_or_B;//用来标识选择有颜色或无颜色时扫雷的界面
	static int iColorSelect = LOWORD(MF_CHECKED);		//用来标识菜单中的颜色选项是否选定
	static int iMarkSelect = LOWORD(MF_CHECKED);      //用来标识菜单中的标记选项是否选定
	static int iSoundSelect = LOWORD(MF_UNCHECKED);   //用来标识菜单中的声音选项是否选定
	static int Mark = 1; //初始时Mark为1,标识允许标记，取消时Mark为0,不允许使用？标记
	
	static HINSTANCE hInst;
	static PAINTSTRUCT ps;
	static HDC hdc;
	static int  Current_Mines;//当前剩余的雷数
	static RECT rect,ShowMine_Rect,Smile_Rect,Time_Rect,Rect_i_j;                         //定义客户区矩形、计雷器区、笑脸区和计时器区
	int x,y,i,j;
	char UserName[3][20];
	char Times[3][10];
	char Buffer[30];
	int CurrentTime =0;	
	FILE *file;
	static BOOL LeftButton,RightButton;//标识按下的是左键还是右键还是左右键都按下
	POINT point;																			//记录鼠标的位置
	switch(message)
	{
	case WM_CREATE: 
		 hInst			 =  ((LPCREATESTRUCT) lparam)->hInstance;
		 hBitMap[0]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_COLOURBRICKS));//彩色的数字和旗子的位图
		 hBitMap[1]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BLACKBRICKS)); //黑白的数字和旗子的位图
		 hBitMap[2]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_REDCOUNTS));   //红色的计数器数字位图
		 hBitMap[3]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BLACKCOUNTS)); //黑白的计数器数字位图
		 hBitMap[4]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_YELLOWSMILES));//黄色笑脸的位图
		 hBitMap[5]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BLACKSMILES));//黑白笑脸的位图

		 SetTimer(hWnd,ID_TIMER,1000,NULL);//设定计时器
		 game_State = 0;
		 timeCount = 0;
		 GetObject(hBitMap[Bricks_C_or_B], sizeof (BITMAP), &bitMap);
		 cxSource  =  bitMap.bmWidth;					//资源位图的宽度
		 cySource  =  bitMap.bmHeight/16 ;             //资源位图的高度

		 Bricks_C_or_B=0;							   //初始时为彩色
		 Counts_C_or_B=2;
		 Smiles_C_or_B=4;

		 cxBorder  =  GetSystemMetrics(SM_CXBORDER);   //边框外围的一个宽度
		 cyBorder  =  GetSystemMetrics(SM_CYBORDER);   //底部边框的高度
		 cyCaption =  GetSystemMetrics(SM_CYCAPTION);  //标题栏的高度
		 cyMenu    =  GetSystemMetrics(SM_CYMENU);	   //菜单栏的高度

		 xBricks   =  9;                               //初始时为9X9
		 yBricks   =  9;
		 Mines	   =  10;
		 Current_Mines = Mines;

		 wndBorderWidth  =  cxBorder*2+12*2+1;              //除了方砖绘制区域以外的宽度
		 wndBorderHeight =  cyBorder+cyCaption+cyMenu+12+63;//除了方砖绘制区域以外的高度
		 Mines_Initialization(Mines,xBricks,yBricks );//初始化雷区
		 
		 //设置初始窗口的大小
		 SetWindowPos(hWnd,
					  HWND_TOP,
					  CW_USEDEFAULT,
					  CW_USEDEFAULT,
					  WndWidth(wndBorderWidth,xBricks,cxSource),
					  WndHeight(wndBorderHeight,yBricks,cySource),
					  SWP_NOMOVE);
		 return 0;
	case WM_COMMAND:
		 switch(LOWORD(wparam))
		 {
			case IDM_START:
				 Mines_Initialization(Mines,xBricks,yBricks);
				 DrawBrick(hWnd,hdc,Smile_Rect,0,hBitMap,Smiles_C_or_B);
				 memset(Map,0,sizeof(Map));
				 game_State = 0;
				 timeCount =0;
				 Current_Mines = Mines;
				 return 0;
			case IDM_MARKED:
				 if(iMarkSelect == LOWORD(MF_CHECKED))
				 {
					 CheckMenuItem(GetMenu(hWnd),IDM_MARKED,MF_UNCHECKED);
					 iMarkSelect = LOWORD(MF_UNCHECKED);
					 Mark = 0;
				 }
				 else {
					 CheckMenuItem(GetMenu(hWnd),IDM_MARKED,MF_CHECKED);
					 iMarkSelect = LOWORD(MF_CHECKED);
					 Mark = 1;
				 }
				 return 0; 
			case IDM_COLOUR:
				 if(iColorSelect==LOWORD(MF_CHECKED))
				 {
					 CheckMenuItem(GetMenu(hWnd),IDM_COLOUR,MF_UNCHECKED);
					 iColorSelect  = LOWORD(MF_UNCHECKED);
					 Bricks_C_or_B = 1;							   //初始时为彩色
					 Counts_C_or_B = 3;
					 Smiles_C_or_B = 5;
				 }
				 else 
				 {
					 CheckMenuItem(GetMenu(hWnd),IDM_COLOUR,MF_CHECKED);
					 iColorSelect  = LOWORD(MF_CHECKED);
					 Bricks_C_or_B = 0;							   //初始时为彩色
					 Counts_C_or_B = 2;
					 Smiles_C_or_B = 4;
				 }
				 //InvalidateRect(hWnd,NULL,TRUE);
				 return 0;
			case IDM_SOUND:
				  if(iSoundSelect == LOWORD(MF_UNCHECKED))
				  {
					   CheckMenuItem(GetMenu(hWnd),IDM_SOUND,MF_CHECKED);
					   Sound = 1;
				  }
				  else {
					   CheckMenuItem(GetMenu(hWnd),IDM_SOUND,MF_UNCHECKED);
					   Sound = 0;
				  }
				  return 0;
			case IDM_HERO_LISTS:
				 DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOG_HEROLIST),hWnd,HeroListProc);
				 return 0;
			case IDM_PRIMARY://初级9*9
			case IDM_MIDDLE://中级16*16
			case IDM_SUPER://高级30*16
			case IDM_USER_DEFINED://玩家自己定义
				 CheckMenuItem (GetMenu(hWnd), iLevelSelect, MF_UNCHECKED) ;
                 iLevelSelect = LOWORD (wparam) ;
                 CheckMenuItem (GetMenu(hWnd), iLevelSelect, MF_CHECKED) ;
				 if(iLevelSelect == IDM_PRIMARY)
				 {
					xBricks  =  9;
					yBricks  =  9;
					Mines	 =  10;
				 }
				 else if(iLevelSelect == IDM_MIDDLE)
				 {
					xBricks  =  16;
					yBricks  =  16;
					Mines	 =  40;
				 }
				 else if(iLevelSelect == IDM_SUPER)
				 {
					xBricks  =  30;
					yBricks  =  16;
					Mines	 =  99;
				 }
				 else if(iLevelSelect == IDM_USER_DEFINED)
				 {
					 DialogBox(hInst,MAKEINTRESOURCE(IDD_USERDEFINEDIALOG),hWnd,UserProc);
				 }
				 Current_Mines = Mines;
				 Mines_Initialization(Mines,xBricks,yBricks);//初始化雷区
				 DrawBrick(hWnd,hdc,Smile_Rect,0,hBitMap,Smiles_C_or_B);
				 memset(Map,0,sizeof(Map));
				 game_State = 0;
				 timeCount =0;
				 SetWindowPos(hWnd,
							  HWND_TOP,
							  CW_USEDEFAULT,
							  CW_USEDEFAULT,
							  WndWidth(wndBorderWidth,xBricks,cxSource),
							  WndHeight(wndBorderHeight,yBricks,cySource),
							  SWP_NOMOVE);
				  return 0;
			case IDM_APPQUIT:
					PostQuitMessage(0);
				 return 0;

			case IDM_CATALOG:
			case IDM_FIND_HELP_TOPIC:
			case IDM_HELP_USED :
				 return 0;
			case IDM_ABOUT :
				 DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUTDIALOG),hWnd,AboutProc);
				 break;
			 }
				break;
			case WM_SIZE:
				 cxClient = LOWORD(lparam);
				 cyClient = HIWORD(lparam);
				 GetClientRect(hWnd,&rect);

				 Mine_Rect.left	= rect.left+11;
				 Mine_Rect.right = rect.right-11;
				 Mine_Rect.top	 = rect.top+61;
				 Mine_Rect.bottom = rect.bottom-11;

				 Time_Rect.left	= rect.right-50;
				 Time_Rect.top	= rect.top+20;
				 Time_Rect.right = rect.right-11;
				 Time_Rect.bottom = rect.top+43;

				 ShowMine_Rect.left = rect.left+11;
				 ShowMine_Rect.top = rect.top+20;
				 ShowMine_Rect.right = rect.left+50;
				 ShowMine_Rect.bottom = rect.top+43;

				 Smile_Rect.left = rect.left+cxClient/2 - 12;
				 Smile_Rect.top = rect.top+20;
				 Smile_Rect.bottom = rect.top+43;
				 Smile_Rect.right = rect.left+cxClient/2+12;
				 //InvalidateRect(hWnd,NULL,FALSE);
				 return 0;
			case WM_PAINT:
				 GetClientRect(hWnd,&rect);
				 hdcDes  = BeginPaint (hWnd, &ps) ;
				 hdcSour = CreateCompatibleDC (hdcDes) ;
				
				 Draw_3D(hWnd,hdcDes,rect);//绘制立体边框
				 Draw_Counts(hWnd,hdcDes,ShowMine_Rect,Current_Mines,hBitMap,Counts_C_or_B);//绘制计雷器
				 Draw_Counts(hWnd,hdcDes,Time_Rect,timeCount,hBitMap,Counts_C_or_B);//绘制计时器
				 DrawBrick(hWnd,hdc,Smile_Rect,4,hBitMap,Smiles_C_or_B);//绘制笑脸 
				
				 //绘制方砖
				 if(game_State == 0)
					 for (y=0; y< yBricks; y += 1)
						 for (x=0 ; x<xBricks; x += 1)
						 {
							  Rect_i_j = GetBricks_i_j_Rect( x,  y,Mine_Rect);
							 DrawBrick(hWnd,hdc,Rect_i_j,0,hBitMap,Bricks_C_or_B);//绘制白方块
						 }
				 if(game_State == 2)DrawBrick(hWnd,hdc,Smile_Rect,1,hBitMap,Smiles_C_or_B);//赢得游戏绘制戴眼镜的笑脸
				 if(game_State == 3)
					 DrawBrick(hWnd,hdc,Smile_Rect,2,hBitMap,Smiles_C_or_B);//绘制哭脸
				 for (y=0; y< yBricks; y += 1)
					for (x=0 ; x<xBricks; x += 1)
					{
						Rect_i_j = GetBricks_i_j_Rect( x,  y,Mine_Rect);
						if(Map[x][y] == 1)		
							DrawBrick(hWnd,hdc,Rect_i_j,15-Bricks[x][y],hBitMap,Bricks_C_or_B);//绘制Bricks[x][y]的内容
						else  if(Map[x][y] == 2)
							DrawBrick(hWnd,hdc,Rect_i_j,1,hBitMap,Bricks_C_or_B);//绘制红旗
						else  if(Map[x][y]==0)
							DrawBrick(hWnd,hdc,Rect_i_j,0,hBitMap,Bricks_C_or_B);//绘制白方块
						else if(Map[x][y]==4)
							DrawBrick(hWnd,hdc,Rect_i_j,2,hBitMap,Bricks_C_or_B);//绘制问号
						else if(Map[x][y] == 3)
							DrawBrick(hWnd,hdc,Rect_i_j,3,hBitMap,Bricks_C_or_B);//绘制踩到的第一颗雷	
						else if(Map[x][y] == 5)
							DrawBrick(hWnd,hdc,Rect_i_j,4,hBitMap,Bricks_C_or_B);//绘制因为标记错误而踩到的雷
					}
				 
				 DeleteDC (hdcSour) ;
				 EndPaint (hWnd, &ps);
				 return 0;
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
				if((wparam & MK_LBUTTON)&&!(wparam & MK_RBUTTON))//如果按下左键则LeftButton为true
				{
					LeftButton = TRUE;
					RightButton = FALSE;
				}
				if(!(wparam & MK_LBUTTON)&&(wparam & MK_RBUTTON))//如果按下左键则RightButton为true
				{
					LeftButton = FALSE;
					RightButton = TRUE;
				}
				if((GetAsyncKeyState(VK_LBUTTON)&0x8000) && (GetAsyncKeyState(VK_RBUTTON)&0x8000))
				{
					LeftButton = TRUE;
					RightButton = TRUE;
				}
				 point.x = LOWORD(lparam);//鼠标点击的位置横坐标
				 point.y = HIWORD(lparam);//鼠标点击的位置纵坐标
				 x = LOWORD(lparam);//鼠标点击的位置横坐标
				 y = HIWORD(lparam);//鼠标点击的位置纵坐标

				 i = (x - Mine_Rect.left)/16; //通过坐标获得点击的是哪个方块，Bricks[i][j]
				 j = (y - Mine_Rect.top)/16;
				 if(game_State !=2&&game_State != 3)
				 {
					 if(LeftButton&&!RightButton)
					 {
							 if(PtInRect(&Smile_Rect,point))
							 {
								Mines_Initialization(Mines,xBricks,yBricks);
								DrawBrick(hWnd,hdc,Smile_Rect,0,hBitMap,Smiles_C_or_B);
								memset(Map,0,sizeof(Map));
								game_State = 0;
								timeCount =0;
								Current_Mines = Mines;
							 }
							if(PtInRect(&rect,point)&&!PtInRect(&Smile_Rect,point))
								DrawBrick(hWnd,hdc,Smile_Rect,3,hBitMap,Smiles_C_or_B);
							if(PtInRect(&Mine_Rect,point))
							{
								if(Bricks[i][j]!=10)
								{
									game_State = 1;
									Mine_Clear(i,j,hWnd,hdc,Mine_Rect,hBitMap,Bricks_C_or_B);
								}
								if(Bricks[i][j] == 10) 
								{
									game_State = 3;
									Map[i][j] = 3;//踩到的第一个雷
									if(Sound == 1)
										PlaySound(TEXT("bomb.wav"),NULL,SND_FILENAME | SND_ASYNC);
									for(y=0;y<yBricks;y++)
										for(x =0;x<xBricks;x++)
										{
											if(Bricks[x][y] == 10&&Map[x][y] != 3)
												Map[x][y] = 1;
											if(Bricks[x][y] !=10 && Map[x][y] == 2)
												Map[x][y] = 5;
										}
								}
							}
					 }
					 if(RightButton&&!LeftButton)
					 {
						 if(PtInRect(&Mine_Rect,point))
						 {
							 if(Map[i][j]==0)
							 {
								 Current_Mines--;
								 Map[i][j]=2;//插旗
								 game_State = 1;
							 }
							 else if(Map[i][j]==2)
							 {
								 if(Mark == 1)//如果允许标记
								 {
									 Map[i][j] = 4;//打上问号标记
									 Current_Mines++; 
								 }
								 else {//否则
									 Map[i][j] =0;//插两次旗子等于没插
									Current_Mines++; 
								 }
								 
							 }
							else if(Map[i][j] == 4)
							{
								Map[i][j] =0;//插两次旗子等于没插
								
							}
						 }
					 }
					 if(LeftButton&&RightButton)
					 {
						Double_Click(i,j,hWnd,hdc,Mine_Rect,hBitMap,Bricks_C_or_B);	 
					 }
				}
				else {
							if((game_State == 3 || game_State == 2 )&&!PtInRect(&Smile_Rect,point))
								return 0;							
							 if(PtInRect(&Smile_Rect,point))
							 {
								Mines_Initialization(Mines,xBricks,yBricks);
								DrawBrick(hWnd,hdc,Smile_Rect,0,hBitMap,Smiles_C_or_B);
								game_State = 0;
								timeCount =0;
								Current_Mines =Mines;
								memset(Map,0,sizeof(Map));
							 }
						}
				if(GameWin())
				{
					game_State = 2;
					file = fopen("HeroList.txt","r+");
					memset(UserName,0,sizeof(UserName));
					memset(Times,0,sizeof(Times));
					memset(Buffer,0,sizeof(Buffer));
					for(i=0;i<3;i++)
					{
						ftell(file);
						fgets(Buffer,30,file);
						sscanf(Buffer,"%s%s",Times[i],UserName[i]);
					}
					fclose(file);
					if(iLevelSelect == IDM_USER_DEFINED)
						MessageBox(NULL,TEXT("恭喜你在自定义游戏中取得了胜利!"),TEXT("恭喜恭喜"),MB_OK);
					else {

							if(iLevelSelect == IDM_PRIMARY)
							{
								CurrentTime = atoi(Times[0]);
							}
							else if(iLevelSelect == IDM_MIDDLE)
							{
								CurrentTime = atoi(Times[1]);
							}
							else if(iLevelSelect == IDM_SUPER)
							{
								CurrentTime = atoi(Times[2]);
							}
				
							if(CurrentTime> timeCount)
							DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOG_HERONAME),hWnd,HeroProc);
							else MessageBox(NULL,TEXT("恭喜你游戏胜利，但是你还没有破纪录哦，请继续加油!"),TEXT("恭喜恭喜"),MB_OK);
					}
				}
				 Sleep(100);
				 InvalidateRect(hWnd,NULL,FALSE);
				 return 0;
			case WM_TIMER:
					if(timeCount == 999)
						return 0;
					if(game_State == 1)
					{
						timeCount++;
						if(Sound == 1)
							PlaySound(TEXT("clock.wav"),NULL, SND_FILENAME | SND_ASYNC) ;
					}
					InvalidateRect(hWnd,NULL,FALSE);
					return 0;
			case WM_DESTROY:
				 DeleteObject(hBitMap);
				 KillTimer(hWnd,ID_TIMER);
				 PostQuitMessage(0);
				 return 0;
		}
	return DefWindowProc(hWnd,message,wparam,lparam);
}
//about对话框处理程序
BOOL CALLBACK AboutProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	{
	case WM_INITDIALOG:
			return TRUE;
	case WM_COMMAND:
		   switch(LOWORD(wparam))
		  {
			case IDOK:
			case IDCANCEL:
				EndDialog(hwnd,0);
				return TRUE;
		}
		   break;
	}

	return FALSE;
}
//用户自定义消息处理函数
BOOL CALLBACK UserProc(HWND hwnd, UINT message, WPARAM wparam,LPARAM lparam)
{
	static int x,y,m;
	switch(message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hwnd,IDC_EDIT_WIDTH,xBricks,TRUE);
		SetDlgItemInt(hwnd,IDC_EDIT_HEIGTH,yBricks,TRUE);
		SetDlgItemInt(hwnd,IDC_EDIT_MINE,Mines,TRUE);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDC_EDIT_HEIGTH:
				y = GetDlgItemInt(hwnd,IDC_EDIT_HEIGTH,NULL,TRUE);
				if(y>=9&&y<=24)
					yBricks = y;
				else if(y<9) yBricks = 9;
				else if(y>24)yBricks = 24;
				return TRUE;
		case IDC_EDIT_WIDTH:
				x = GetDlgItemInt(hwnd,IDC_EDIT_WIDTH,NULL,TRUE);
				if(x>=9&&x<=30)
					xBricks = x;
				else if(x<9) xBricks = 9;
				else if(x>30)xBricks = 30;
				return TRUE;
		case IDC_EDIT_MINE:
				m = GetDlgItemInt(hwnd,IDC_EDIT_MINE,NULL,TRUE);
				if(m>=10&&m<=(xBricks-1)*(yBricks-1))
					Mines = m;
				else if(m<10)Mines = 10;
				else Mines = (xBricks-1)*(yBricks-1);
				return TRUE;
		case IDC_BUTTONOK:
				yBricks = GetDlgItemInt(hwnd,IDC_EDIT_HEIGTH,NULL,TRUE);
				xBricks = GetDlgItemInt(hwnd,IDC_EDIT_WIDTH,NULL,TRUE);
				Mines   =  GetDlgItemInt(hwnd,IDC_EDIT_MINE,NULL,TRUE);
		case IDC_BUTTONCANCLE:
		case IDCANCEL:
			EndDialog(hwnd,0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
//游戏胜利弹出填写玩家姓名的消息函数
BOOL CALLBACK HeroProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	char UserName[3][20];
	char Times[3][10];
	char Buffer[30];
	int CurrentTime =0;	
	int i=0;
	FILE *file;
	switch(message)
	{
	case WM_INITDIALOG:	
	    SetDlgItemText(hwnd,IDC_EDIT_HERONAME,TEXT("匿名"));
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDOK:
		case IDCANCEL:
			GetDlgItemText(hwnd,IDC_EDIT_HERONAME,WinName,sizeof(WinName));
			
			file = fopen("HeroList.txt","r+");
			memset(UserName,0,sizeof(UserName));
			memset(Times,0,sizeof(Times));
			memset(Buffer,0,sizeof(Buffer));
			for(i=0;i<3;i++)//读取文件中的信息到相应的数组中
			{
				ftell(file);
				fgets(Buffer,30,file);
				sscanf(Buffer,"%s%s",Times[i],UserName[i]);
			}
			fclose(file);
			if(iLevelSelect == IDM_PRIMARY)
			{
				CurrentTime = atoi(Times[0]);
				if(timeCount< CurrentTime)
				{
					itoa(timeCount,Times[0],10);
					strcpy(UserName[0],WinName);
				}
			}
			else if(iLevelSelect == IDM_MIDDLE)
			{
				CurrentTime = atoi(Times[1]);
				if(timeCount< CurrentTime)
				{
					itoa(timeCount,Times[1],10);
					strcpy(UserName[1],WinName);
				}
			}
			else if(iLevelSelect == IDM_SUPER)
			{
				CurrentTime = atoi(Times[2]);
				if(timeCount< CurrentTime)
				{
					itoa(timeCount,Times[2],10);
					strcpy(UserName[2],WinName);
				}
			}
			file = fopen("HeroList.txt","w+");
			for(i=0;i<3;i++)//重新写入更新后的信息
			{
				fseek(file,ftell(file),SEEK_SET);
				fputs(Times[i],file);
				fputs("  ",file);
				fputs(UserName[i],file);
				fputc('\n',file);
			}
			fclose(file);
			EndDialog(hwnd,0);
			DialogBox(NULL ,MAKEINTRESOURCE(IDD_DIALOG_HEROLIST),hwnd,HeroListProc);//弹出英雄榜对话框
			return TRUE;
		}
		break;
	}

	return FALSE;
} 
//扫雷英雄榜处理函数
BOOL CALLBACK HeroListProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	char UserName[3][20];
	char Times[3][10];
	char Buffer[30];
	int CurrentTime =0;	
	int i=0;
	FILE *file;
	file = fopen("HeroList.txt","r+");
	memset(UserName,0,sizeof(UserName));
	memset(Times,0,sizeof(Times));
	memset(Buffer,0,sizeof(Buffer));
	for(i=0;i<3;i++)
	{
		ftell(file);
		fgets(Buffer,30,file);
		sscanf(Buffer,"%s%s",Times[i],UserName[i]);
	}
	fclose(file);
	
	switch(message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwnd,IDC_PRINAME,UserName[0]);
		SetDlgItemText(hwnd,IDC_PRITIME,Times[0]);

		SetDlgItemText(hwnd,IDC_MIDNAME,UserName[1]);
		SetDlgItemText(hwnd,IDC_MIDTIME,Times[1]);

		SetDlgItemText(hwnd,IDC_SUPNAME,UserName[2]);
		SetDlgItemText(hwnd,IDC_SUPTIME,Times[2]);
		
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDCREST:
			for(i=0;i<3;i++)
			{
				strcpy(UserName[i],"匿名");
				strcpy(Times[i],"999");
			}
			SetDlgItemText(hwnd,IDC_PRINAME,UserName[0]);
			SetDlgItemText(hwnd,IDC_PRITIME,Times[0]);

			SetDlgItemText(hwnd,IDC_MIDNAME,UserName[1]);
			SetDlgItemText(hwnd,IDC_MIDTIME,Times[1]);

			SetDlgItemText(hwnd,IDC_SUPNAME,UserName[2]);
			SetDlgItemText(hwnd,IDC_SUPTIME,Times[2]);
			file = fopen("HeroList.txt","w+");
			for(i=0;i<3;i++)
			{
				fseek(file,ftell(file),SEEK_SET);
				fputs(Times[i],file);
				fputs("  ",file);
				fputs(UserName[i],file);
				fputc('\n',file);
			}
			fclose(file);
			return TRUE;
		case IDCOK:
		case IDCANCEL:
			
			EndDialog(hwnd,0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
//绘制具有立体边框的函数
void Draw_3D(HWND hwnd,HDC hdc,RECT rect )
{
		int i;
		int x[2] = { rect.left+10,rect.right-10,};//x坐标
		int y[4] = {rect.top+10,rect.top+50,rect.top+60,rect.bottom-10};//y坐标
		hdc	= GetDC(hwnd);
		SelectObject(hdc,GetStockObject(DC_PEN));       //选取画笔
		SetDCPenColor(hdc,RGB(127,127,127));			//设置画笔颜色
		
		//绘制深颜色的线
		for(i=0;i<2;i++)
		{
			MoveToEx(hdc,x[1],y[0]+i,NULL);
			LineTo(hdc,x[0],y[0]+i);
			LineTo(hdc,x[0],y[1]+i);

			MoveToEx(hdc,x[1],y[2]+i,NULL);
			LineTo(hdc,x[0],y[2]+i);
			LineTo(hdc,x[0],y[3]+i);
		}

		SetDCPenColor(hdc,RGB(255,255,255));			//重新设置画笔的颜色
		//绘制浅颜色的线
		for(i=0;i<2;i++)
		{
			MoveToEx(hdc,x[1],y[0]+i,NULL);
			LineTo(hdc,x[1],y[1]+i);
			LineTo(hdc,x[0],y[1]+i);

			MoveToEx(hdc,x[1],y[2]+i,NULL);
			LineTo(hdc,x[1],y[3]+i);
			LineTo(hdc,x[0],y[3]+i);
		}
		ReleaseDC(hwnd,hdc);
}
//计算窗口的宽度
int WndWidth(int wndBorderWidth,int xBricks,int cxSource)
{
	return wndBorderWidth+cxSource*xBricks;
}

//计算窗口的高度
int WndHeight(int wndBorderHeight,int yBricks,int cySource)
{
	return wndBorderHeight+yBricks*cySource;
}
//绘制方块或者笑脸的函数
void DrawBrick(HWND hwnd, HDC hdc,RECT rect,  int brickIndex, HBITMAP hBitMap[],int bmpIndex)
{
	BITMAP bitMap;
	HDC hdcSour;
	int x,y;
	hdc = GetDC(hwnd);
	hdcSour = CreateCompatibleDC(hdc);
	SelectObject (hdcSour, hBitMap[bmpIndex]) ;
	GetObject(hBitMap[bmpIndex], sizeof (BITMAP), &bitMap);
	x = bitMap.bmWidth;
	y = x;
	BitBlt (hdc, rect.left, rect.top, x, y, hdcSour, 0, x*brickIndex, SRCCOPY) ;
	
	DeleteDC (hdcSour) ;
	ReleaseDC(hwnd, hdc);
	
}
//初始化雷区函数实现
void Mines_Initialization( int Mines,int xBricks,int yBricks )
{
	int i,j,count;
	memset(Bricks,0,sizeof(Bricks));
	srand((unsigned)time(NULL));
	for(count=0;count<Mines;)
	{
		i=rand()%xBricks;
		j=rand()%yBricks;
		if(Bricks[i][j]!=10)
		{
			Bricks[i][j]=10;
			count++;
		 }
	 }
    for(i=0;i<xBricks;i++)
		for(j=0;j<yBricks;j++)
			if(Bricks[i][j]!=10)Bricks[i][j]=Judge_AroundMines(i,j,xBricks,yBricks);
}
//判断一个块周围的雷数
int		Judge_AroundMines(int xindex,int yindex,int xBricks,int yBricks)
{
	int i,j,mines=0;
	int Lx,Ly,Rx,Ry;//左上角和右下角两个坐标
	Lx = xindex-1;
	Ly = yindex-1;
	Rx = xindex+1;
	Ry = yindex+1;
	if(Lx<0)Lx=0;
	if(Ly<0)Ly=0;
	if(Rx>xBricks-1)Rx=xBricks-1;
	if(Ry>yBricks-1)Ry=yBricks-1;
	for(i=Lx;i<=Rx;i++)
		for(j=Ly;j<=Ry;j++)
		if(Bricks[i][j]==10)mines++;
	return mines;
}
//绘制计雷器和计时器
void   Draw_Counts(HWND hwnd,HDC hdc,RECT rect,int Mine,HBITMAP hBitMap[],int index)
{
	BITMAP bitMap;
	int x,y,mine;
	HDC hdcSour;
	hdc = GetDC(hwnd);
	hdcSour = CreateCompatibleDC (hdc) ;
	SelectObject (hdcSour, hBitMap[index]);
	GetObject(hBitMap[index], sizeof (BITMAP), &bitMap);
	if(Mine >= 0)//如果当前雷数为正数，即:标记的数目小于总的雷数
	{
		mine = Mine;
		y = 100;
		for(x = 0;x < 3;x++)
		{
			BitBlt(hdc,rect.left+x*bitMap.bmWidth,rect.top,bitMap.bmWidth,bitMap.bmHeight/12,hdcSour,0,(bitMap.bmHeight/12)*(11-mine/y),SRCCOPY);
			mine = mine % y;
			y = y/10;
		}
	}
	else {//当前雷数为负数的时候，即:旗子插的太多导致显示为负		
		mine = -Mine;
		if(mine >99)
			mine = mine/10;
		y = 10;
		BitBlt(hdc,rect.left,rect.top,bitMap.bmWidth,bitMap.bmHeight/12,hdcSour,0,0,SRCCOPY);//第一个绘制成负号
		for(x = 1;x<3;x++)
		{
			BitBlt(hdc,rect.left+x*bitMap.bmWidth,rect.top,bitMap.bmWidth,bitMap.bmHeight/12,hdcSour,0,(bitMap.bmHeight/12)*(11-mine/y),SRCCOPY);
			mine = mine % y;
			y = y/10;
		}
	}
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcSour);
}
//根据i,j获得Bricks[i][j]的矩形区域
RECT   GetBricks_i_j_Rect(int i, int j,RECT rect)
{
	RECT Rect_i_j;
	Rect_i_j.left = rect.left+i*16;
	Rect_i_j.right = rect.left+(i+1)*16;
	Rect_i_j.top = rect.top+j*16;
	Rect_i_j.bottom = rect.top+(j+1)*16;
	return Rect_i_j;
}
//当点击到空白时显示其周围最大可显示的区域
int	 Mine_Clear(int i,int j,HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int bmpIndex)
{
	int Lx,Ly,Rx,Ry,x,y;
	Lx = i-1;
	Ly = j-1;
	Rx = i+1;
	Ry = j+1;
	if(Lx<0)Lx=0;
	if(Ly<0)Ly=0;
	if(Rx>xBricks-1)Rx=xBricks-1;
	if(Ry>yBricks-1)Ry=yBricks-1;
	
	if(Map[i][j] == 0)
	{
		if(Bricks[i][j] != 10)
			//DrawBrick(hwnd,hdc,GetBricks_i_j_Rect(i,j,rect),15-Bricks[i][j],hBitMap,bmpIndex);
		Map[i][j] = 1;
	}
	else return 0;
	if(Bricks[i][j] == 0)
	{
		for(y=Ly;y<=Ry;y++)
			for(x=Lx;x<=Rx;x++)
					Mine_Clear(x,y,hwnd,hdc,rect,hBitMap,bmpIndex);
	}
	return 0;
}
//鼠标双击时某个已显示方块时的效果
void Double_Click(int i,int j,HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int bmpIndex)
{
	int Lx,Ly,Rx,Ry,x,y;
	int Around_Mine;//一个方块周围的雷数
	Lx = i-1;
	Ly = j-1;
	Rx = i+1;
	Ry = j+1;
	if(Lx<0)Lx=0;
	if(Ly<0)Ly=0;
	if(Rx>xBricks-1)Rx=xBricks-1;
	if(Ry>yBricks-1)Ry=yBricks-1;
	Around_Mine = Judge_AroundMines(i,j,xBricks,yBricks);
	for(y=Ly;y<=Ry;y++)
		for(x=Lx;x<=Rx;x++)
			if(Bricks[x][y] == 10&&Map[x][y] == 2)
				Around_Mine --;
	if(Around_Mine)
	{
		for(y=Ly;y<=Ry;y++)
			for(x=Lx;x<=Rx;x++)
				if(Map[x][y] == 0)
				{	
					if(Bricks[x][y] == 0)
					Mine_Clear(x,y,hwnd,hdc,Mine_Rect,hBitMap,bmpIndex);
					DrawBrick(hwnd,hdc,GetBricks_i_j_Rect(x,y,rect),15,hBitMap,bmpIndex);
				}
				else if(Map[x][y] == 4)
					DrawBrick(hwnd,hdc,GetBricks_i_j_Rect(x,y,rect),6,hBitMap,bmpIndex);
				else if(Map[x][y] == 2&&Bricks[x][y] != 10)
				{
					game_State = 3;
					Map[x][y] = 5;
				}
		if(game_State == 3)
		{
			if(Sound == 1)
				PlaySound(TEXT("bomb.wav"),NULL,SND_FILENAME | SND_ASYNC);
			//当前位置范围内的雷的确切位置
			for(y=Ly;y<=Ry;y++)
				for(x=Lx;x<=Rx;x++)
					if(Bricks[x][y] == 10)
						Map[x][y] = 3;
			//把所有的雷都显示出来
			for(y=0;y<yBricks;y++)
				for(x=0;x<xBricks;x++)
					if(Bricks[x][y] == 10 && Map[x][y] == 0)
						Map[x][y] = 1;
					else if(Map[x][y] == 2&& Bricks[x][y] !=10)
						Map[x][y] = 5;
		}
	}
	else 
	{
		for(y=Ly;y<=Ry;y++)
			for(x=Lx;x<=Rx;x++)
				if(Map[x][y] == 0)
				{	
					if(Bricks[x][y] == 0)
						Mine_Clear(x,y,hwnd,hdc,Mine_Rect,hBitMap,bmpIndex);
					DrawBrick(hwnd,hdc,GetBricks_i_j_Rect(x,y,rect),15-Bricks[x][y],hBitMap,bmpIndex);
					Map[x][y]=1;
				}
	} 
}
//判断游戏是否胜利
int GameWin()
{
	int i,j;
	int flag =0;//插旗子方块数、未翻开的方块数和有问号标记的方块数的总和
	int num =0;//已经翻开的方块数

	for(i = 0;i<yBricks;i++)
		for(j=0;j<xBricks;j++)
		{
			if(Map[i][j] == 2 || Map[i][j] == 0 || Map[i][j] == 4)flag++;
			if(Map[i][j] == 1)num++;
		}
	if(flag == Mines&&(flag+num) == xBricks*yBricks)
		return 1;
	return 0;
}