/*----------------------------------------------
   ɨ��-- Version using Common Dialog Box
                (c) ��ï��, 2014
  ----------------------------------------------*/
#include<Windows.h>
#include<time.h>
#include<stdio.h>
#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")
#include"resource.h"
#define ID_TIMER 1
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);//������Ϣ������
void	  Draw_3D(HWND ,HDC ,RECT);//���ƾ�������б߿���
int		  WndWidth(int ,int ,int );				//���㴰�ڿ�Ⱥ���
int		  WndHeight(int,int,int);					//���㴰�ڸ߶Ⱥ���
void	   DrawBrick(HWND hwnd, HDC hdc,RECT rect,  int brickIndex, HBITMAP hBitMap[],int bmpIndex);//���Ʒ����Ц������ 
void    Mines_Initialization(int,int,int  );		 //��ʼ��������������
int		   Judge_AroundMines(int ,int,int,int);//�ж�һ������Χ����������
void    Draw_Counts(HWND hwnd,HDC hdc,RECT rect,int Mine,HBITMAP hBitMap[],int index);//���Ƽ������ͼ�ʱ������
RECT   GetBricks_i_j_Rect(int i, int j,RECT rect);//����i,j���Bricks[i][j]�ľ�������
int	 Mine_Clear(int i,int j,HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int bmpIndex);//��������հ�ʱ��ʾ����Χ������ʾ������
void Double_Click(int i,int j,HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int bmpIndex);//���˫��ʱĳ������ʾ����ʱ��Ч��
int GameWin();//�ж���Ϸ�Ƿ�ȡ��ʤ��
BOOL CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);//About�Ի��� ����Ϣ����
BOOL CALLBACK UserProc(HWND, UINT, WPARAM,LPARAM );//�û��Զ�����Ϣ����
BOOL CALLBACK HeroProc(HWND, UINT, WPARAM,LPARAM );//��Ϸʤ��������д�����������Ϣ����
BOOL CALLBACK HeroListProc(HWND, UINT, WPARAM,LPARAM );//ɨ��Ӣ�۰���Ϣ������

//ȫ�ֱ����Ķ���
static  TCHAR szAppName[]=TEXT("ɨ��--��ï�� ѧ��:2013E8009061062");
static int xBricks,yBricks,Mines;					//��������������Ϸ���ĸ���,Mine��ʾ������
static int Bricks[30][24]={0};//��¼�������׵Ĳ����Լ�����Χ������
static int Map[30][24]={0};//�����Ӧ�����״̬(0��1��2��3��4��5)�����ǿհ���Ϊ0�����������Map[i][j]=1;���һ�����Map[i][j]=2;������ȵ���(��һ����)��Map[i][j]=3;���һ���ʾ�ʺ���Map[i][j]=4;����Ϊ���������ȵ�����Ϊ5
static TCHAR WinName[20];//����Ƽ�¼�����������
static int iLevelSelect = IDM_PRIMARY;				//������ʶ�˵��е���Ϸ�����ѡ��
static int timeCount;														//��ʱ��
static int game_State;												//��ʶ��Ϸδ��ʼ0����ʼ1������(��ʤ����2���ȵ��׽���3)����״̬
static RECT Mine_Rect; //������������
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
							 WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,//�������
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
	static HBITMAP hBitMap[6] ;							//λͼ�������
	static int cxClient,cyClient,cxSource,cySource;		//cxClient,cyClient�ͻ������ȺͿ�ȣ�cxSource��cySource��Դ�ļ���λ��	
	static int wndBorderWidth,wndBorderHeight;			//���ڵĳ��ȺͿ��
	static int cxBorder,cyBorder,cyCaption,cyMenu;      //��ʾ���ڵ���Χ�߿�Ĳ���������߿��һ����ȣ��ײ��߿�ĸ߶ȣ��������ĸ߶ȣ��˵����ĸ߶�                   
	static int Bricks_C_or_B,Counts_C_or_B,Smiles_C_or_B;//������ʶѡ������ɫ������ɫʱɨ�׵Ľ���
	static int iColorSelect = LOWORD(MF_CHECKED);		//������ʶ�˵��е���ɫѡ���Ƿ�ѡ��
	static int iMarkSelect = LOWORD(MF_CHECKED);      //������ʶ�˵��еı��ѡ���Ƿ�ѡ��
	static int iSoundSelect = LOWORD(MF_UNCHECKED);   //������ʶ�˵��е�����ѡ���Ƿ�ѡ��
	static int Mark = 1; //��ʼʱMarkΪ1,��ʶ�����ǣ�ȡ��ʱMarkΪ0,������ʹ�ã����
	
	static HINSTANCE hInst;
	static PAINTSTRUCT ps;
	static HDC hdc;
	static int  Current_Mines;//��ǰʣ�������
	static RECT rect,ShowMine_Rect,Smile_Rect,Time_Rect,Rect_i_j;                         //����ͻ������Ρ�����������Ц�����ͼ�ʱ����
	int x,y,i,j;
	char UserName[3][20];
	char Times[3][10];
	char Buffer[30];
	int CurrentTime =0;	
	FILE *file;
	static BOOL LeftButton,RightButton;//��ʶ���µ�����������Ҽ��������Ҽ�������
	POINT point;																			//��¼����λ��
	switch(message)
	{
	case WM_CREATE: 
		 hInst			 =  ((LPCREATESTRUCT) lparam)->hInstance;
		 hBitMap[0]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_COLOURBRICKS));//��ɫ�����ֺ����ӵ�λͼ
		 hBitMap[1]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BLACKBRICKS)); //�ڰ׵����ֺ����ӵ�λͼ
		 hBitMap[2]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_REDCOUNTS));   //��ɫ�ļ���������λͼ
		 hBitMap[3]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BLACKCOUNTS)); //�ڰ׵ļ���������λͼ
		 hBitMap[4]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_YELLOWSMILES));//��ɫЦ����λͼ
		 hBitMap[5]   =  LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BLACKSMILES));//�ڰ�Ц����λͼ

		 SetTimer(hWnd,ID_TIMER,1000,NULL);//�趨��ʱ��
		 game_State = 0;
		 timeCount = 0;
		 GetObject(hBitMap[Bricks_C_or_B], sizeof (BITMAP), &bitMap);
		 cxSource  =  bitMap.bmWidth;					//��Դλͼ�Ŀ��
		 cySource  =  bitMap.bmHeight/16 ;             //��Դλͼ�ĸ߶�

		 Bricks_C_or_B=0;							   //��ʼʱΪ��ɫ
		 Counts_C_or_B=2;
		 Smiles_C_or_B=4;

		 cxBorder  =  GetSystemMetrics(SM_CXBORDER);   //�߿���Χ��һ�����
		 cyBorder  =  GetSystemMetrics(SM_CYBORDER);   //�ײ��߿�ĸ߶�
		 cyCaption =  GetSystemMetrics(SM_CYCAPTION);  //�������ĸ߶�
		 cyMenu    =  GetSystemMetrics(SM_CYMENU);	   //�˵����ĸ߶�

		 xBricks   =  9;                               //��ʼʱΪ9X9
		 yBricks   =  9;
		 Mines	   =  10;
		 Current_Mines = Mines;

		 wndBorderWidth  =  cxBorder*2+12*2+1;              //���˷�ש������������Ŀ��
		 wndBorderHeight =  cyBorder+cyCaption+cyMenu+12+63;//���˷�ש������������ĸ߶�
		 Mines_Initialization(Mines,xBricks,yBricks );//��ʼ������
		 
		 //���ó�ʼ���ڵĴ�С
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
					 Bricks_C_or_B = 1;							   //��ʼʱΪ��ɫ
					 Counts_C_or_B = 3;
					 Smiles_C_or_B = 5;
				 }
				 else 
				 {
					 CheckMenuItem(GetMenu(hWnd),IDM_COLOUR,MF_CHECKED);
					 iColorSelect  = LOWORD(MF_CHECKED);
					 Bricks_C_or_B = 0;							   //��ʼʱΪ��ɫ
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
			case IDM_PRIMARY://����9*9
			case IDM_MIDDLE://�м�16*16
			case IDM_SUPER://�߼�30*16
			case IDM_USER_DEFINED://����Լ�����
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
				 Mines_Initialization(Mines,xBricks,yBricks);//��ʼ������
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
				
				 Draw_3D(hWnd,hdcDes,rect);//��������߿�
				 Draw_Counts(hWnd,hdcDes,ShowMine_Rect,Current_Mines,hBitMap,Counts_C_or_B);//���Ƽ�����
				 Draw_Counts(hWnd,hdcDes,Time_Rect,timeCount,hBitMap,Counts_C_or_B);//���Ƽ�ʱ��
				 DrawBrick(hWnd,hdc,Smile_Rect,4,hBitMap,Smiles_C_or_B);//����Ц�� 
				
				 //���Ʒ�ש
				 if(game_State == 0)
					 for (y=0; y< yBricks; y += 1)
						 for (x=0 ; x<xBricks; x += 1)
						 {
							  Rect_i_j = GetBricks_i_j_Rect( x,  y,Mine_Rect);
							 DrawBrick(hWnd,hdc,Rect_i_j,0,hBitMap,Bricks_C_or_B);//���ư׷���
						 }
				 if(game_State == 2)DrawBrick(hWnd,hdc,Smile_Rect,1,hBitMap,Smiles_C_or_B);//Ӯ����Ϸ���ƴ��۾���Ц��
				 if(game_State == 3)
					 DrawBrick(hWnd,hdc,Smile_Rect,2,hBitMap,Smiles_C_or_B);//���ƿ���
				 for (y=0; y< yBricks; y += 1)
					for (x=0 ; x<xBricks; x += 1)
					{
						Rect_i_j = GetBricks_i_j_Rect( x,  y,Mine_Rect);
						if(Map[x][y] == 1)		
							DrawBrick(hWnd,hdc,Rect_i_j,15-Bricks[x][y],hBitMap,Bricks_C_or_B);//����Bricks[x][y]������
						else  if(Map[x][y] == 2)
							DrawBrick(hWnd,hdc,Rect_i_j,1,hBitMap,Bricks_C_or_B);//���ƺ���
						else  if(Map[x][y]==0)
							DrawBrick(hWnd,hdc,Rect_i_j,0,hBitMap,Bricks_C_or_B);//���ư׷���
						else if(Map[x][y]==4)
							DrawBrick(hWnd,hdc,Rect_i_j,2,hBitMap,Bricks_C_or_B);//�����ʺ�
						else if(Map[x][y] == 3)
							DrawBrick(hWnd,hdc,Rect_i_j,3,hBitMap,Bricks_C_or_B);//���Ʋȵ��ĵ�һ����	
						else if(Map[x][y] == 5)
							DrawBrick(hWnd,hdc,Rect_i_j,4,hBitMap,Bricks_C_or_B);//������Ϊ��Ǵ�����ȵ�����
					}
				 
				 DeleteDC (hdcSour) ;
				 EndPaint (hWnd, &ps);
				 return 0;
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
				if((wparam & MK_LBUTTON)&&!(wparam & MK_RBUTTON))//������������LeftButtonΪtrue
				{
					LeftButton = TRUE;
					RightButton = FALSE;
				}
				if(!(wparam & MK_LBUTTON)&&(wparam & MK_RBUTTON))//������������RightButtonΪtrue
				{
					LeftButton = FALSE;
					RightButton = TRUE;
				}
				if((GetAsyncKeyState(VK_LBUTTON)&0x8000) && (GetAsyncKeyState(VK_RBUTTON)&0x8000))
				{
					LeftButton = TRUE;
					RightButton = TRUE;
				}
				 point.x = LOWORD(lparam);//�������λ�ú�����
				 point.y = HIWORD(lparam);//�������λ��������
				 x = LOWORD(lparam);//�������λ�ú�����
				 y = HIWORD(lparam);//�������λ��������

				 i = (x - Mine_Rect.left)/16; //ͨ�������õ�������ĸ����飬Bricks[i][j]
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
									Map[i][j] = 3;//�ȵ��ĵ�һ����
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
								 Map[i][j]=2;//����
								 game_State = 1;
							 }
							 else if(Map[i][j]==2)
							 {
								 if(Mark == 1)//���������
								 {
									 Map[i][j] = 4;//�����ʺű��
									 Current_Mines++; 
								 }
								 else {//����
									 Map[i][j] =0;//���������ӵ���û��
									Current_Mines++; 
								 }
								 
							 }
							else if(Map[i][j] == 4)
							{
								Map[i][j] =0;//���������ӵ���û��
								
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
						MessageBox(NULL,TEXT("��ϲ�����Զ�����Ϸ��ȡ����ʤ��!"),TEXT("��ϲ��ϲ"),MB_OK);
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
							else MessageBox(NULL,TEXT("��ϲ����Ϸʤ���������㻹û���Ƽ�¼Ŷ�����������!"),TEXT("��ϲ��ϲ"),MB_OK);
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
//about�Ի��������
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
//�û��Զ�����Ϣ������
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
//��Ϸʤ��������д�����������Ϣ����
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
	    SetDlgItemText(hwnd,IDC_EDIT_HERONAME,TEXT("����"));
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
			for(i=0;i<3;i++)//��ȡ�ļ��е���Ϣ����Ӧ��������
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
			for(i=0;i<3;i++)//����д����º����Ϣ
			{
				fseek(file,ftell(file),SEEK_SET);
				fputs(Times[i],file);
				fputs("  ",file);
				fputs(UserName[i],file);
				fputc('\n',file);
			}
			fclose(file);
			EndDialog(hwnd,0);
			DialogBox(NULL ,MAKEINTRESOURCE(IDD_DIALOG_HEROLIST),hwnd,HeroListProc);//����Ӣ�۰�Ի���
			return TRUE;
		}
		break;
	}

	return FALSE;
} 
//ɨ��Ӣ�۰�����
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
				strcpy(UserName[i],"����");
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
//���ƾ�������߿�ĺ���
void Draw_3D(HWND hwnd,HDC hdc,RECT rect )
{
		int i;
		int x[2] = { rect.left+10,rect.right-10,};//x����
		int y[4] = {rect.top+10,rect.top+50,rect.top+60,rect.bottom-10};//y����
		hdc	= GetDC(hwnd);
		SelectObject(hdc,GetStockObject(DC_PEN));       //ѡȡ����
		SetDCPenColor(hdc,RGB(127,127,127));			//���û�����ɫ
		
		//��������ɫ����
		for(i=0;i<2;i++)
		{
			MoveToEx(hdc,x[1],y[0]+i,NULL);
			LineTo(hdc,x[0],y[0]+i);
			LineTo(hdc,x[0],y[1]+i);

			MoveToEx(hdc,x[1],y[2]+i,NULL);
			LineTo(hdc,x[0],y[2]+i);
			LineTo(hdc,x[0],y[3]+i);
		}

		SetDCPenColor(hdc,RGB(255,255,255));			//�������û��ʵ���ɫ
		//����ǳ��ɫ����
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
//���㴰�ڵĿ��
int WndWidth(int wndBorderWidth,int xBricks,int cxSource)
{
	return wndBorderWidth+cxSource*xBricks;
}

//���㴰�ڵĸ߶�
int WndHeight(int wndBorderHeight,int yBricks,int cySource)
{
	return wndBorderHeight+yBricks*cySource;
}
//���Ʒ������Ц���ĺ���
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
//��ʼ����������ʵ��
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
//�ж�һ������Χ������
int		Judge_AroundMines(int xindex,int yindex,int xBricks,int yBricks)
{
	int i,j,mines=0;
	int Lx,Ly,Rx,Ry;//���ϽǺ����½���������
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
//���Ƽ������ͼ�ʱ��
void   Draw_Counts(HWND hwnd,HDC hdc,RECT rect,int Mine,HBITMAP hBitMap[],int index)
{
	BITMAP bitMap;
	int x,y,mine;
	HDC hdcSour;
	hdc = GetDC(hwnd);
	hdcSour = CreateCompatibleDC (hdc) ;
	SelectObject (hdcSour, hBitMap[index]);
	GetObject(hBitMap[index], sizeof (BITMAP), &bitMap);
	if(Mine >= 0)//�����ǰ����Ϊ��������:��ǵ���ĿС���ܵ�����
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
	else {//��ǰ����Ϊ������ʱ�򣬼�:���Ӳ��̫�ർ����ʾΪ��		
		mine = -Mine;
		if(mine >99)
			mine = mine/10;
		y = 10;
		BitBlt(hdc,rect.left,rect.top,bitMap.bmWidth,bitMap.bmHeight/12,hdcSour,0,0,SRCCOPY);//��һ�����Ƴɸ���
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
//����i,j���Bricks[i][j]�ľ�������
RECT   GetBricks_i_j_Rect(int i, int j,RECT rect)
{
	RECT Rect_i_j;
	Rect_i_j.left = rect.left+i*16;
	Rect_i_j.right = rect.left+(i+1)*16;
	Rect_i_j.top = rect.top+j*16;
	Rect_i_j.bottom = rect.top+(j+1)*16;
	return Rect_i_j;
}
//��������հ�ʱ��ʾ����Χ������ʾ������
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
//���˫��ʱĳ������ʾ����ʱ��Ч��
void Double_Click(int i,int j,HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int bmpIndex)
{
	int Lx,Ly,Rx,Ry,x,y;
	int Around_Mine;//һ��������Χ������
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
			//��ǰλ�÷�Χ�ڵ��׵�ȷ��λ��
			for(y=Ly;y<=Ry;y++)
				for(x=Lx;x<=Rx;x++)
					if(Bricks[x][y] == 10)
						Map[x][y] = 3;
			//�����е��׶���ʾ����
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
//�ж���Ϸ�Ƿ�ʤ��
int GameWin()
{
	int i,j;
	int flag =0;//�����ӷ�������δ�����ķ����������ʺű�ǵķ��������ܺ�
	int num =0;//�Ѿ������ķ�����

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