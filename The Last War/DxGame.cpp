/*----------------------------------------------
  DxGame.cpp - "The Last War" main source file
	(c) 1999-2002 Vlad Bologa & Emil Butiri
------------------------------------------------*/

#include "stdafx.h"
#include "Bmp.h"

#include "map.h"
#include "unitplane.h"
#include "struccenter.h"

#include <stdio.h>
#include <streams.h>
#include <string.h>

#include <ddraw.h>
#include <dinput.h>

#define  VERSION "v0.3.3b"

#define KEYDOWN(name,key) (name[key] & 0x80)
#define WM_GRAPHNOTIFY  WM_USER+13
#define WM_BEGINGAME	WM_USER+14
#define RELEASE(x) { if (x) x->Release(); x = NULL; }

#define PLAYING			TRUE
#define STOPPED			FALSE

#define LBT_BLACK		0
#define LBT_TRANSPARENT	1
#define LM_STATIC		0
#define LM_DINAMIC		1

#define TILESIZE		160
#define MAX_TILES		2

#define CCENTER			1

#define LWU_AIR			1	
#define LWU_PLANE		1
#define LWU_F15			2

#define UNINIT_MENU		1
#define UNINIT_GAME		2		
enum GameState
{
	MAIN_MENU = 0,
	GAME_ACTIVE,
	GAME_PAUSED,
	NEW_GAME
};

GameState State = MAIN_MENU;
BOOL FilmState = STOPPED;
BOOL IsReading = FALSE;

RECT ScreenSize;

CBmp bmp;
CBmp MenuBack, MINewGame[2], MIExit[2], MILoadGame[2], MIOptions[2];
CBmp ComputerAnim[51];
CBmp Load, Cursor;

LONG      evCode;
LONG      evParam1;
LONG      evParam2;

int INIT_ERROR=0;
char buffer[256];
int NewGameCounter = 0;

int MouseSensitivity = 200;
int iResX, iResY;

//DirectX & DirectX Media COM Objects
IBaseFilter   *pif   = NULL;
IGraphBuilder *pigb  = NULL;
IMediaControl *pimc  = NULL;
IMediaEventEx *pimex = NULL;
IVideoWindow  *pivw  = NULL;

LPDIRECTDRAW7 pDD7 = NULL;
LPDIRECTDRAWSURFACE7 pDDPrimary = NULL;
LPDIRECTDRAWSURFACE7 pDDBackBuffer = NULL;
LPDIRECTINPUT pDI = NULL;
LPDIRECTINPUTDEVICE pDIKeyboard = NULL;
LPDIRECTINPUTDEVICE pDIMouse = NULL;
LPDIRECTDRAWCLIPPER pDDClipper = NULL;

LPDIRECTDRAWSURFACE7 pDDCursor = NULL;
LPDIRECTDRAWSURFACE7 pDDOffscreen = NULL;
LPDIRECTDRAWSURFACE7 pDDPanel = NULL;
LPDIRECTDRAWSURFACE7 pDDTile[MAX_TILES];
LPDIRECTDRAWSURFACE7 pDDMenuBegin, pDDMenuOpt;
LPDIRECTDRAWSURFACE7 pDDMenuBeginTXT, pDDMenuBeginTXTSel, pDDMenuOptTXTSel, pDDMenuExitTXTSel;
LPDIRECTDRAWSURFACE7 pDDSprite120x90[66];
LPDIRECTDRAWSURFACE7 pDDSprite185x160;
LPDIRECTDRAWSURFACE7 pDDSprite120x100[4];
LPDIRECTDRAWSURFACE7 pDDSprite170x160;
LPDIRECTDRAWSURFACE7 pDDUnitSelection;
LPDIRECTDRAWSURFACE7 pDDCCenterSelected;
LPDIRECTDRAWSURFACE7 pDDMenuButtonPressed, pDDMenuButtonOver;
LPDIRECTDRAWSURFACE7 pDDPlaneButton, pDDPlaneButtonPressed;
LPDIRECTDRAWSURFACE7 pDDF15Button, pDDF15ButtonPressed;

//Function prototypes
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
BOOL StartGame(HWND);
BOOL DirectDrawInit(int,int,HWND);
BOOL DirectInputInit(HWND);
BOOL TileInScreen(RECT);
BOOL UpdateMainMenu(HWND);
BOOL CreateGameOffscreenSurfaces();
BOOL CreateMenuOffscreenSurfaces();
void DirectDrawUnInit(int);
void DirectInputUnInit();
int UpdateGame(HWND);
void LoadMenuFiles();
void PlayFile(LPSTR, HWND);
void PlayFileDDEx(LPSTR);
void Loading(int, int);
void DarkenScreen();

//***************List Class*****************

class CBmpList
{
	CBmp Frame;
public:
	BOOL Draw(const HDC &hdc, int x = 0, int y = 0);
	BOOL Load(LPSTR filename);
	CBmpList *next;
};

BOOL CBmpList::Draw(const HDC &hdc, int x, int y)
{
	if (Frame.Draw(hdc, x, y)) return TRUE;
		else return FALSE;
}

BOOL CBmpList::Load(LPSTR filename)
{
	if (Frame.Load(filename)) return TRUE;
		else return FALSE;
}

//***************List Class*****************

CBmpList *BGActual, *BGNext, *BGFirst, *OptActual, *OptNext, *OptFirst;

//***************Engine Class***************

class GameEngine
{
	BOOL MouseOnPanel();
	int CurentX, CurentY;
	int mouse_x, mouse_y, fmouse_x, fmouse_y;
	BOOL bLeftBtnPressed, RightButtonPressed;
	BOOL IsSelecting, WaitSelection;
	BOOL fLMBPressed, oldLMBPressed, oldRMBPressed;
	BOOL MenuButtonPressed, PlaneButtonPressed, PlaneButtonPressedOld, bF15BtnP, bF15BtnPOld;
	CMap Map;
	int UnitCount;
	int StructSelType;
	
	CUnit *Unit, *first, *temp;
	CStructure *Struct;
public:
	void GetMouseCoords(int &x, int &y);
	void ShowMouse();
	void CorrectCoords();
	BOOL Update(int Reserved = 0);
	BOOL UpdateTerrain(int x = 0, int y = 0);
	BOOL Load(int Level = 0, int Reserved = 0);
	void LoadTerrainTiles(int tileset = 0);
	void AddUnit(int iType, int iSubType, CStructure *Parent);
};

BOOL GameEngine::Update(int Reserved)
{
	HDC hdc;
	int m_x, m_y, iNrSel=0;
	POINT pCursor;
	RECT rButton;
	HRESULT hr;
	static HPEN hPen=CreatePen(PS_SOLID, 1, RGB(20,200,40));
	static int add=0;
	BOOL bMouseOnPanel;

	GetMouseCoords(m_x, m_y);
	pDIKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
	if (!bLeftBtnPressed) 
		MenuButtonPressed=PlaneButtonPressed=bF15BtnP=FALSE;
	bMouseOnPanel=MouseOnPanel();
	
//***********Scrolling***********
	if (!IsSelecting)
	{
		if (m_x == iResX)
			CurentX+=10;
		if (m_x == 0) CurentX-=10;
		if (m_y == iResY) 
			CurentY+=10;
		if (m_y == 0) CurentY-=10;
	}
	CorrectCoords();
	UpdateTerrain(CurentX, CurentY);
//********End of Scrolling********

//***********Selection***********
	if (IsSelecting&&(!bLeftBtnPressed))
	{

		POINT pt;
		LONG tmp;
		RECT rc, r_Unit, temp, r_Structure;

		Unit=first;
		while (Unit)
		{

			pt.x=Unit->GetX()-CurentX;
			pt.y=Unit->GetY()-CurentY;
			
			rc.top=fmouse_y;
			rc.left=fmouse_x;
			rc.bottom=mouse_y;
			rc.right=mouse_x;

			r_Unit.top=pt.y;
			r_Unit.left=pt.x;
			r_Unit.bottom=pt.y+90;
			r_Unit.right=pt.x+120;
			
			if (rc.top>rc.bottom){ tmp=rc.top; rc.top=rc.bottom; rc.bottom=tmp;}
			if (rc.left>rc.right){ tmp=rc.left; rc.left=rc.right; rc.right=tmp;}

			if (IntersectRect(&temp, &rc, &r_Unit)) 
			{
				Unit->Select(TRUE);
				iNrSel++;
			}
			else Unit->Select(FALSE);
			Unit=Unit->next;
		}
		
		pt.x=Struct->GetX()-CurentX;
		pt.y=Struct->GetY()-CurentY;
			
		rc.top=fmouse_y;
		rc.left=fmouse_x;
		rc.bottom=mouse_y;
		rc.right=mouse_x;

		r_Structure.top=pt.y;
		r_Structure.left=pt.x;
		r_Structure.bottom=pt.y+160;
		r_Structure.right=pt.x+185;
					
		if (rc.top>rc.bottom){ tmp=rc.top; rc.top=rc.bottom; rc.bottom=tmp;}
		if (rc.left>rc.right){ tmp=rc.left; rc.left=rc.right; rc.right=tmp;}

		if (rc.right>704) rc.right=705;

		if (IntersectRect(&temp, &rc, &r_Structure)&&(!iNrSel))
		{
			Struct->Select(TRUE);
			StructSelType=Struct->GetType();
		}
		else
		{
			Struct->Select(FALSE);
			StructSelType=0;
		}

		IsSelecting=FALSE;
		WaitSelection=FALSE;
	}

	if (WaitSelection&&!bLeftBtnPressed)
	{
		BOOL Usel=FALSE,Ssel=FALSE;
		POINT pt;
		RECT r_Structure;
		
		Unit=first;
		if (Unit) while (Unit->next) Unit=Unit->next;
		while (Unit)
		{
			RECT r_Unit;

			pt.x=mouse_x;
			pt.y=mouse_y;

			r_Unit.top=Unit->GetY()-CurentY;
			r_Unit.left=Unit->GetX()-CurentX;
			r_Unit.bottom=r_Unit.top+90;
			r_Unit.right=r_Unit.left+120;

			int CursorOnUnit=0;
			if (PtInRect(&r_Unit, pt))
			{
				int relx, rely, poz;
				DDSURFACEDESC2 sDesc;
				sDesc.dwSize=sizeof(sDesc);
				int frame=Unit->GetCurrentFrame();
				
				relx=mouse_x-r_Unit.left;
				rely=mouse_y-r_Unit.top;
				if (pDDSprite120x90[frame]->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
				{
					PBYTE mem=(PBYTE) sDesc.lpSurface;
					poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

					if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnUnit=1;
					pDDSprite120x90[frame]->Unlock(NULL);
				}
			}
			
			if (CursorOnUnit)
			{ 	
				if (!Usel)
				{
					Unit->Select(TRUE);
					Usel=TRUE;
					iNrSel++;
				}
				else Unit->Select(FALSE);
			}
			else Unit->Select(FALSE);
			Unit=Unit->prev;
		}
		
		pt.x=mouse_x;
		pt.y=mouse_y;

		r_Structure.top=Struct->GetY()-CurentY;
		r_Structure.left=Struct->GetX()-CurentX;
		r_Structure.bottom=r_Structure.top + 160;
		r_Structure.right=r_Structure.left + 185;

		if (PtInRect(&r_Structure, pt)&&(!iNrSel))
		{
			Struct->Select(TRUE);
			StructSelType=Struct->GetType();
		}
		else
		{
			Struct->Select(FALSE);
			StructSelType=0;
		}

		WaitSelection=FALSE;
	}

	if (bLeftBtnPressed&&oldLMBPressed&&(!fLMBPressed)&&!IsSelecting&&!bMouseOnPanel) WaitSelection=TRUE;
	if (WaitSelection) if (mouse_x!=fmouse_x&&mouse_y!=fmouse_y) 
	{
		IsSelecting=TRUE;
		WaitSelection=FALSE;
	}
//***********End of Selection***********

	RECT DestRect;
	static int bframe=0, sgn=1;

	SetRect(&DestRect, 830-CurentX, 640-CurentY, 1000-CurentX, 800-CurentY);
	pDDBackBuffer->Blt(&DestRect, pDDSprite170x160, NULL, DDBLT_WAIT, NULL);
	
	SetRect(&DestRect, 1000-CurentX, 700-CurentY, 1120-CurentX, 800-CurentY);
	pDDBackBuffer->Blt(&DestRect, pDDSprite120x100[bframe/10], NULL, DDBLT_WAIT, NULL);
	bframe+=sgn;
	if (bframe==39) sgn=-1;
	if (bframe==0) sgn=1;

	SetRect(&DestRect, Struct->GetX()-CurentX, Struct->GetY()-CurentY, Struct->GetX()+185-CurentX, Struct->GetY()+160-CurentY);
	if (Struct->Selected()) pDDBackBuffer->Blt(&DestRect, pDDCCenterSelected, NULL, DDBLT_WAIT, NULL);
	else pDDBackBuffer->Blt(&DestRect, pDDSprite185x160, NULL, DDBLT_WAIT, NULL);

	Unit=first;
	while (Unit)
	{
		if (!RightButtonPressed&&oldRMBPressed&&Unit->Selected()&&!bMouseOnPanel) Unit->SetDestination(CurentX+m_x-60+rand()%10, CurentY+m_y-45+rand()%10);
		Unit->Update();
		SetRect(&DestRect, Unit->GetX()-CurentX, Unit->GetY()-CurentY, Unit->GetX()+120-CurentX, Unit->GetY()+90-CurentY);
		if (Unit->Selected())
			pDDBackBuffer->Blt(&DestRect, pDDUnitSelection, NULL, DDBLT_WAIT|DDBLT_KEYSRC, NULL);
		switch (Unit->GetSubType())
		{
		case 1:
			pDDBackBuffer->Blt(&DestRect, pDDSprite120x90[Unit->GetCurrentFrame()], NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
			break;
		case 2:
			pDDBackBuffer->Blt(&DestRect, pDDSprite120x90[Unit->GetCurrentFrame()+33], NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
		Unit=Unit->next;
	}	

	if (IsSelecting)
	{
		int tmp_fmouse_x=fmouse_x, tmp_mouse_x=mouse_x;
		
		pDDBackBuffer->GetDC(&hdc);
		SelectObject(hdc, hPen);
		SelectObject(hdc, (HBRUSH) GetStockObject(NULL_BRUSH));
		if (tmp_fmouse_x>704) tmp_fmouse_x=705;
		if (tmp_mouse_x>704) tmp_mouse_x=705;
		Rectangle(hdc,tmp_fmouse_x, fmouse_y, tmp_mouse_x, mouse_y);
		pDDBackBuffer->ReleaseDC(hdc);
	}
	
	SetRect(&DestRect,iResX-125,0,iResX,iResY);
	hr = pDDBackBuffer->Blt(&DestRect, pDDPanel, NULL, DDBLT_WAIT|DDBLT_KEYSRC, NULL);
	if (hr!=DD_OK) return FALSE;

	pCursor.x=mouse_x;
	pCursor.y=mouse_y;
	SetRect(&rButton,710,540,710 + 80,540 + 45);
	if (PtInRect(&rButton, pCursor))
		if (bLeftBtnPressed)
		{
			if (!oldLMBPressed) MenuButtonPressed=TRUE;
			if (MenuButtonPressed)
				pDDBackBuffer->Blt(&rButton, pDDMenuButtonPressed, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rButton, pDDMenuButtonOver, NULL, DDBLT_WAIT, NULL);
	
	if (StructSelType==CCENTER)
	{
		//Add Plane Button
		SetRect(&rButton,715,130,715+80,130+60);
		if (PtInRect(&rButton, pCursor))
		{
			if (bLeftBtnPressed)
			{
				if (!oldLMBPressed) PlaneButtonPressed=TRUE;
				if (PlaneButtonPressed) pDDBackBuffer->Blt(&rButton, pDDPlaneButtonPressed, NULL, DDBLT_WAIT, NULL);
				else pDDBackBuffer->Blt(&rButton, pDDPlaneButton, NULL, DDBLT_WAIT, NULL);
			}
			else pDDBackBuffer->Blt(&rButton, pDDPlaneButton, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rButton, pDDPlaneButton, NULL, DDBLT_WAIT, NULL);
		if (PtInRect(&rButton,pCursor)&&PlaneButtonPressedOld&&(!bLeftBtnPressed))
		{		
			if (!add)
			{
				AddUnit(LWU_AIR, LWU_PLANE, Struct);
				add=1;
			}
		}
		else add=0;

		//Add F15 Button
		SetRect(&rButton,715,185,715+80,185+60);
		if (PtInRect(&rButton, pCursor))
		{
			if (bLeftBtnPressed)
			{
				if (!oldLMBPressed) bF15BtnP=TRUE;
				if (bF15BtnP) pDDBackBuffer->Blt(&rButton, pDDF15ButtonPressed, NULL, DDBLT_WAIT, NULL);
				else pDDBackBuffer->Blt(&rButton, pDDF15Button, NULL, DDBLT_WAIT, NULL);
			}
			else pDDBackBuffer->Blt(&rButton, pDDF15Button, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rButton, pDDF15Button, NULL, DDBLT_WAIT, NULL);
		if (PtInRect(&rButton,pCursor)&&bF15BtnPOld&&(!bLeftBtnPressed))
		{		
			if (!add)
			{
				AddUnit(LWU_AIR, LWU_F15, Struct);
				add=1;
			}
		}
		else add=0;
	}
		
	/*pDDBackBuffer->GetDC(&hdc);
	SetTextAlign(hdc, TA_BOTTOM | TA_RIGHT);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255,255,255));
	if (bMouseOnPanel) TextOut(hdc, 800, 600, "OnPanel", 7);
	else TextOut(hdc, 800, 600, "NotOnPanel", 10);
	pDDBackBuffer->ReleaseDC(hdc);*/

	ShowMouse();
	if (!IsSelecting&&!WaitSelection)
	{
		fmouse_x=mouse_x;
		fmouse_y=mouse_y;
		fLMBPressed=oldLMBPressed;
	}
	oldLMBPressed=bLeftBtnPressed;
	oldRMBPressed=RightButtonPressed;
	PlaneButtonPressedOld=PlaneButtonPressed;
	bF15BtnPOld=bF15BtnP;

	pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	
	if (KEYDOWN(buffer, DIK_ESCAPE))
	{
		DeleteObject(hPen);
		return FALSE;
	}

	if (KEYDOWN(buffer, DIK_DELETE))
	{
		int change=0;
		do
		{
			Unit=first;
			change=0;
			if (first)
			{
				if (first->Selected()&&(!first->next))
				{
					delete Unit;
					first=NULL;
					change=1;
				}
				else if (first->Selected())
				{
					Unit=Unit->next;
					Unit->prev=NULL;
					delete first;
					first=Unit;
					change=1;
				}
				else
				{
					while ((Unit->next)&&(!Unit->Selected())) Unit=Unit->next;
					if (Unit->next)
					{
						temp=Unit;
						Unit=Unit->prev;
						Unit->next=temp->next;
						delete temp;
						temp=Unit;
						Unit=Unit->next;
						Unit->prev=temp;
						change=1;
					}
					else if (Unit->Selected())
					{
						temp=Unit->prev;
						temp->next=NULL;
						delete Unit;
						Unit=NULL;
						change=1;
					}
				}
				if (change) UnitCount--;
			}
		}
		while (change);
	}
	return TRUE;
}

void GameEngine::AddUnit(int iType, int iSubType, CStructure *Parent)
{
	if (!first)
	{
		first=(CUnitPlane *) new CUnitPlane;
		first->SetPosition(320,200);
		first->SetDestination(320,200);
		first->SetSubType(iSubType);
		first->Select(FALSE);
		first->next=NULL;
		first->prev=NULL;
	}
	else
	{
		temp=(CUnitPlane *) new CUnitPlane;
		temp->SetPosition(320,200);
		temp->SetDestination(320,200);
		temp->SetSubType(iSubType);
		temp->Select(FALSE);
		Unit=first;
		while (Unit->next) Unit=Unit->next;
		Unit->next=temp;
		temp->prev=Unit;
		temp->next=NULL;
	}
	UnitCount++;
}

BOOL GameEngine::UpdateTerrain(int x, int y)
{
	for (int i = 0; i < Map.GetSizeY(); i++)
		for (int j = 0; j < Map.GetSizeX(); j++)
		{
			RECT DestRect;
			SetRect(&DestRect, (TILESIZE*j)-x, (TILESIZE*i)-y, (TILESIZE*j+TILESIZE)-x, (TILESIZE*i+TILESIZE)-y);
			if (TileInScreen(DestRect))
				pDDBackBuffer->Blt(&DestRect, pDDTile[Map.GetTerrainType(i,j)], NULL, DDBLT_WAIT, NULL);
		}
	return TRUE;
}

void GameEngine::CorrectCoords()
{
	if (CurentX < 0) CurentX = 0;
	if (CurentX > (Map.GetSizeX() * TILESIZE - iResX+95)) CurentX = Map.GetSizeX() * TILESIZE - iResX+95;
	if (CurentY < 0) CurentY = 0;
	if (CurentY > (Map.GetSizeY() * TILESIZE - iResY)) CurentY = Map.GetSizeY() * TILESIZE - iResY;
}

BOOL GameEngine::MouseOnPanel()
{
	if (mouse_x>705) return TRUE;
	if (mouse_x>675)
	{
		BOOL bMoP=FALSE;
		int relx, rely, poz;
		DDSURFACEDESC2 sDesc;
		sDesc.dwSize=sizeof(sDesc);
						
		relx=mouse_x-675;
		rely=mouse_y;
		if (pDDPanel->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
		{
			PBYTE mem=(PBYTE) sDesc.lpSurface;
			poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

			if (mem[poz]!=0&&mem[poz+1]!=0) bMoP=TRUE;;
			pDDPanel->Unlock(NULL);
		}
		if (bMoP) return TRUE;
	}
	return FALSE;
}

void GameEngine::LoadTerrainTiles(int tileset)
{
	CBmp TerrainType[10];
	HDC hdc;

	TerrainType[0].Load("data\\Tiles\\tile01.bmp");
	TerrainType[1].Load("data\\Tiles\\tile02.bmp");

	for (int i = 0; i < MAX_TILES; i++)
	{
		pDDTile[i]->GetDC(&hdc);
		TerrainType[i].Draw(hdc);
		pDDTile[i]->ReleaseDC(hdc);
	}
}

void GameEngine::GetMouseCoords(int &x, int &y)
{
	static int m_x = 320, m_y  = 160;
	DIMOUSESTATE dims;

	HRESULT hr = pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
	if (FAILED(hr)) PostQuitMessage(0);
	
	m_x+=dims.lX * MouseSensitivity / 100;
	m_y+=dims.lY * MouseSensitivity / 100;

	if (m_x < 0) m_x = 0;
	if (m_x > iResX) m_x = iResX;
	if (m_y < 0) m_y = 0;
	if (m_y > iResY) m_y = iResY;
	
	x = m_x; y = m_y;
	mouse_x = m_x; mouse_y = m_y;
	if (dims.rgbButtons[0] & 0x80) bLeftBtnPressed=TRUE;
		else bLeftBtnPressed=FALSE;
	if (dims.rgbButtons[1] & 0x80) RightButtonPressed=TRUE;
		else RightButtonPressed=FALSE;
}

void GameEngine::ShowMouse()
{
	RECT DestRect;
	
	SetRect(&DestRect, mouse_x, mouse_y, mouse_x + 32, mouse_y + 32);
	HRESULT hr = pDDBackBuffer->Blt(&DestRect, pDDCursor, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	if (FAILED(hr)) PostQuitMessage(0);
}

GameEngine::Load(int Level, int Reserved)
{
	CBmp dBar, Pyramid, lball[4], Uselect, Sselect, USprite, SSprite, MenuButton;
	HDC hdc;
	
	CurentX = CurentY = StructSelType = 0;
	IsSelecting=FALSE;
	WaitSelection=FALSE;	
	Map.Load(Level);
	LoadTerrainTiles();	
	fLMBPressed=oldLMBPressed=oldRMBPressed=FALSE;
	MenuButtonPressed=PlaneButtonPressed=PlaneButtonPressedOld=bF15BtnP=bF15BtnPOld=FALSE;

	Struct=(CStructCCenter *) new CStructCCenter;
	Struct->SetPosition(160,160);
	Struct->next=NULL;
	Struct->prev=NULL;

	first=(CUnitPlane *) new CUnitPlane;
	first->SetPosition(400,300);
	first->SetDestination(600,400);
	first->SetSubType(LWU_PLANE);
	first->Select(TRUE);
	first->next=NULL;
	first->prev=NULL;
	UnitCount=1;

	lball[0].Load("data\\Structures\\ball1.bmp");
	lball[1].Load("data\\Structures\\ball2.bmp");
	lball[2].Load("data\\Structures\\ball3.bmp");
	lball[3].Load("data\\Structures\\ball4.bmp");

	Uselect.Load("data\\Interface\\Uselect.bmp");
	Sselect.Load("data\\Structures\\ccentersel.bmp");
	dBar.Load("data\\Interface\\lpanel.bmp");
	Pyramid.Load("data\\Structures\\pyramid.bmp");

	MenuButton.Load("data\\interface\\mb01.bmp");
	pDDMenuButtonPressed->GetDC(&hdc);
	MenuButton.Draw(hdc);
	pDDMenuButtonPressed->ReleaseDC(hdc);

	MenuButton.Load("data\\interface\\mb02.bmp");
	pDDMenuButtonOver->GetDC(&hdc);
	MenuButton.Draw(hdc);
	pDDMenuButtonOver->ReleaseDC(hdc);

	MenuButton.Load("data\\interface\\pb01.bmp");
	pDDPlaneButton->GetDC(&hdc);
	MenuButton.Draw(hdc);
	pDDPlaneButton->ReleaseDC(hdc);

	MenuButton.Load("data\\interface\\pb02.bmp");
	pDDPlaneButtonPressed->GetDC(&hdc);
	MenuButton.Draw(hdc);
	pDDPlaneButtonPressed->ReleaseDC(hdc);

	MenuButton.Load("data\\interface\\pb01.bmp");
	pDDPlaneButton->GetDC(&hdc);
	MenuButton.Draw(hdc);
	pDDPlaneButton->ReleaseDC(hdc);

	MenuButton.Load("data\\interface\\fb01.bmp");
	pDDF15Button->GetDC(&hdc);
	MenuButton.Draw(hdc);
	pDDF15Button->ReleaseDC(hdc);

	MenuButton.Load("data\\interface\\fb02.bmp");
	pDDF15ButtonPressed->GetDC(&hdc);
	MenuButton.Draw(hdc);
	pDDF15ButtonPressed->ReleaseDC(hdc);

	MenuButton.Load("data\\interface\\pb02.bmp");
	pDDPlaneButtonPressed->GetDC(&hdc);
	MenuButton.Draw(hdc);
	pDDPlaneButtonPressed->ReleaseDC(hdc);

	pDDUnitSelection->GetDC(&hdc);
	Uselect.Draw(hdc);
	pDDUnitSelection->ReleaseDC(hdc);

	pDDCCenterSelected->GetDC(&hdc);
	Sselect.Draw(hdc);
	pDDCCenterSelected->ReleaseDC(hdc);

	pDDSprite170x160->GetDC(&hdc);
	Pyramid.Draw(hdc);
	pDDSprite170x160->ReleaseDC(hdc);
	
	pDDPanel->GetDC(&hdc);
	dBar.Draw(hdc);
	pDDPanel->ReleaseDC(hdc);

	SSprite.Load("data\\structures\\CCenter.bmp");
	pDDSprite185x160->GetDC(&hdc);
	if (FAILED(SSprite.Draw(hdc))) return FALSE;
	pDDSprite185x160->ReleaseDC(hdc);

	for (int i=0; i<4; i++)
	{
		pDDSprite120x100[i]->GetDC(&hdc);
		lball[i].Draw(hdc);
		pDDSprite120x100[i]->ReleaseDC(hdc);
	}

	for (i=0; i<=32; i++)
	{
		char buffer[256];
		sprintf(buffer, "data\\Units\\Plane\\plane%d.bmp", i);
		USprite.Load(buffer);

		pDDSprite120x90[i]->GetDC(&hdc);
		if (FAILED(USprite.Draw(hdc))) return FALSE;
		pDDSprite120x90[i]->ReleaseDC(hdc);
	}

	for (i=33; i<=65; i++)
	{
		char buffer[256];
		sprintf(buffer, "data\\Units\\F15\\plane%d.bmp", i-33);
		USprite.Load(buffer);

		pDDSprite120x90[i]->GetDC(&hdc);
		if (FAILED(USprite.Draw(hdc))) return FALSE;
		pDDSprite120x90[i]->ReleaseDC(hdc);
	}

	return TRUE;
}

//***************Engine Class***************

GameEngine Engine;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
     {
     static char szAppName[] = "The Last War";
	 MSG         msg;
     WNDCLASSEX  wndclass;
	 HRESULT hr;
	 HWND hwnd;

	 CoInitialize(NULL);
	 hr=DirectInputCreate(hInstance,DIRECTINPUT_VERSION, &pDI,NULL);

	 wndclass.cbSize        = sizeof (wndclass);
     wndclass.style         = CS_HREDRAW | CS_VREDRAW;
     wndclass.lpfnWndProc   = WndProc;
     wndclass.cbClsExtra    = 0;
     wndclass.cbWndExtra    = 0;
     wndclass.hInstance     = hInstance;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
     wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
     wndclass.lpszMenuName  = NULL;
     wndclass.lpszClassName = szAppName;
     wndclass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);

     RegisterClassEx (&wndclass);
     hwnd = CreateWindow (szAppName,szAppName,WS_POPUP,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),NULL,NULL,hInstance,NULL);
	 
	 if (hr!=DI_OK)
	 {
		INIT_ERROR=1;
		MessageBox(hwnd,"Fatal error: Could not initialize input. The program cannot continue.","Error",MB_OK | MB_ICONEXCLAMATION);
		PostQuitMessage(0);
	 }

     ShowWindow (hwnd, iCmdShow);
     UpdateWindow (hwnd);

	 while (GetMessage (&msg, NULL, 0, 0))
          {
          TranslateMessage (&msg);
          DispatchMessage (&msg);
          }
	CoUninitialize();
    return msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
		//PlayFile("data\\Video\\Mini-Intro.avi", hwnd);
		SendMessage(hwnd, WM_BEGINGAME, 0,0);
		return 0;

	case WM_BEGINGAME:
		StartGame(hwnd);
		PostQuitMessage(0);
		return 0;
	
	case WM_GRAPHNOTIFY:
		HRESULT hr;
		while (SUCCEEDED(pimex->GetEvent(&evCode, &evParam1, &evParam2, 0)))
              {
                hr = pimex->FreeEventParams(evCode, evParam1, evParam2);
                if ((EC_COMPLETE == evCode) || (EC_USERABORT == evCode))
                  {
					SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

					pivw->put_FullScreenMode(OAFALSE);
					pivw->put_Visible(OAFALSE);

                    RELEASE(pivw);
                    RELEASE(pif);
                    RELEASE(pigb);
                    RELEASE(pimc);
                    RELEASE(pimex);

                    FilmState = STOPPED;
					SendMessage(hwnd, WM_BEGINGAME, 0,0);
					break;
                  }
              }
		return 0;

	case WM_DESTROY:
		PostQuitMessage (0);
		return 0;
	}
	return DefWindowProc (hwnd, iMsg, wParam, lParam);
}


//Begin the GAME
//////////////////////////////////////////////////////////////////

BOOL TileInScreen(RECT tile)
{
	BOOL InRect = FALSE;
	
	POINT p1 = {tile.left, tile.top};
	POINT p2 = {tile.right, tile.bottom};
	POINT p3 = {tile.right - TILESIZE, tile.bottom};
	POINT p4 = {tile.left + TILESIZE, tile.top};

	if (PtInRect(&ScreenSize, p1)) InRect = TRUE;
	if (PtInRect(&ScreenSize, p2)) InRect = TRUE;
	if (PtInRect(&ScreenSize, p3)) InRect = TRUE;
	if (PtInRect(&ScreenSize, p4)) InRect = TRUE;
	
	if (InRect) return TRUE;
	else return FALSE;
}

BOOL DirectInputInit(HWND hwnd)
{
	HRESULT hr;
	
	hr = pDI->CreateDevice(GUID_SysKeyboard,&pDIKeyboard,NULL);
	if (hr!=DI_OK)
	{
		MessageBox(hwnd,"Error while initializing keyboard.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	hr = pDIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (hr!=DI_OK)
	{
		MessageBox(hwnd,"Error while initializing keyboard.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	hr = pDIKeyboard->SetCooperativeLevel(hwnd,DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (hr!=DI_OK)
	{
		MessageBox(hwnd,"Error while initializing keyboard.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	hr = pDIKeyboard->Acquire();
	if (hr!=DI_OK)
	{
		MessageBox(hwnd,"Error while acquiring keyboard.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	
	//Mouse
	hr = pDI->CreateDevice(GUID_SysMouse,&pDIMouse,NULL);
	if (hr!=DI_OK)
	{
		MessageBox(hwnd,"Error while initializing mouse.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	hr = pDIMouse->SetDataFormat(&c_dfDIMouse);
	if (hr!=DI_OK)
	{
		MessageBox(hwnd,"Error while initializing mouse.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	hr = pDIMouse->SetCooperativeLevel(hwnd,DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (hr!=DI_OK)
	{
		MessageBox(hwnd,"Error while initializing mouse.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	hr = pDIMouse->Acquire();
	if (hr!=DI_OK)
	{
		MessageBox(hwnd,"Error while acquiring mouse.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	ShowCursor(FALSE);
	return TRUE;
}

BOOL DirectDrawInit(int rx,int ry,HWND hwnd)
{
	HRESULT hr = 0;
	
	iResX=rx;
	iResY=ry;
	SetRect(&ScreenSize,0,0,rx-95,ry);

	//DirectDraw object creation
	hr = DirectDrawCreateEx(NULL, (void **) &pDD7, IID_IDirectDraw7, NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating DirectDraw object. You must have DirectX 7 installed.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	hr = pDD7->SetCooperativeLevel(hwnd,DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE);
	if (hr!=DD_OK) 
	{
		MessageBox(hwnd,"Error while setting DirectDraw Cooperative level.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	hr = pDD7->SetDisplayMode(rx,ry,16,0,0);
	if (hr!=DD_OK) 
	{
		MessageBox(hwnd,"Error while setting display mode.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	//Primary DirectDrawSurface object creation
	DDSURFACEDESC2 Primary;
	DDSCAPS2  BackBuffer;

	ZeroMemory(&Primary, sizeof(Primary));
	ZeroMemory(&BackBuffer, sizeof(BackBuffer));
	Primary.dwSize = sizeof(Primary);

	Primary.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	Primary.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
	Primary.dwBackBufferCount = 1;
	BackBuffer.dwCaps = DDSCAPS_BACKBUFFER;

	hr = pDD7->CreateSurface(&Primary,&pDDPrimary,NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating primary surface. You might have less than 1 MB of video memory...","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	hr = pDDPrimary->GetAttachedSurface(&BackBuffer, &pDDBackBuffer);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating back buffer. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	//Creating Clipper Object
	PRGNDATA pRgnData;
	RGNDATAHEADER rgndh;
	RECT clip;
	PBYTE pMem;

	pRgnData = (PRGNDATA) new BYTE[sizeof(RGNDATAHEADER) + sizeof(RECT)];
	pMem = (PBYTE) pRgnData;
	ZeroMemory(pRgnData, sizeof(RGNDATAHEADER) + sizeof(RECT));

	rgndh.dwSize = sizeof(RGNDATAHEADER);
	rgndh.iType = RDH_RECTANGLES;
	rgndh.nCount = 1;
	rgndh.nRgnSize = sizeof(RECT);
	rgndh.rcBound.right = 0;
	rgndh.rcBound.top = 0;
	rgndh.rcBound.left = rx+1;
	rgndh.rcBound.bottom = ry+1;

	SetRect(&clip, 0, 0, rx+1, ry+1);
	CopyMemory(pMem, &rgndh, sizeof(rgndh));
	pMem += sizeof(rgndh);
	CopyMemory(pMem, &clip, sizeof(RECT));

	hr = pDD7->CreateClipper(0, &pDDClipper, NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating the clipper object. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	hr = pDDBackBuffer->SetClipper(pDDClipper);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating the clipper object. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	
	hr = pDDClipper->SetClipList(pRgnData, 0);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating the clipper object. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL CreateMenuOffscreenSurfaces()
{
	DDSURFACEDESC2 Offscreen;
	DDCOLORKEY key;

	ZeroMemory(&Offscreen, sizeof(DDSURFACEDESC2));
	Offscreen.dwSize=sizeof(DDSURFACEDESC2);
	key.dwColorSpaceLowValue=0;
	key.dwColorSpaceHighValue=0;

	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 640;
	Offscreen.dwHeight = 480;
	pDD7->CreateSurface(&Offscreen,&pDDOffscreen,NULL);
	pDDOffscreen->SetColorKey(DDCKEY_SRCBLT,&key);

	ZeroMemory(&Offscreen, sizeof(DDSURFACEDESC2));
	Offscreen.dwSize=sizeof(DDSURFACEDESC2);
	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 32;
	Offscreen.dwHeight = 32;
	pDD7->CreateSurface(&Offscreen, &pDDCursor, NULL);
	pDDCursor->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 220;
	Offscreen.dwHeight = 220;
	pDD7->CreateSurface(&Offscreen,&pDDMenuBegin,NULL);

	Offscreen.dwWidth = 170;
	Offscreen.dwHeight = 150;
	pDD7->CreateSurface(&Offscreen,&pDDMenuOpt,NULL);

	Offscreen.dwWidth = 227;
	Offscreen.dwHeight = 42;
	pDD7->CreateSurface(&Offscreen, &pDDMenuBeginTXT, NULL);

	Offscreen.dwWidth = 232;
	Offscreen.dwHeight = 48;
	pDD7->CreateSurface(&Offscreen, &pDDMenuBeginTXTSel, NULL);

	Offscreen.dwWidth = 170;
	Offscreen.dwHeight = 50;
	pDD7->CreateSurface(&Offscreen, &pDDMenuOptTXTSel, NULL);

	Offscreen.dwWidth = 100;
	Offscreen.dwHeight = 50;
	pDD7->CreateSurface(&Offscreen, &pDDMenuExitTXTSel, NULL);

	return TRUE;
}

BOOL CreateGameOffscreenSurfaces()
{
	DDSURFACEDESC2 Offscreen;
	DDCOLORKEY key;
	
	ZeroMemory(&Offscreen, sizeof(DDSURFACEDESC2));
	Offscreen.dwSize=sizeof(DDSURFACEDESC2);

	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = TILESIZE;
	Offscreen.dwHeight = TILESIZE;
	key.dwColorSpaceLowValue=0;
	key.dwColorSpaceHighValue=0;
	for (int i = 0; i < MAX_TILES; i++)
	{
		pDD7->CreateSurface(&Offscreen,&pDDTile[i],NULL);
		pDDTile[i]->SetColorKey(DDCKEY_SRCBLT,&key);
	}
	
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 120;
	Offscreen.dwHeight = 90;
	pDD7->CreateSurface(&Offscreen, &pDDUnitSelection, NULL);
	pDDUnitSelection->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.dwWidth = 185;
	Offscreen.dwHeight = 160;
	pDD7->CreateSurface(&Offscreen, &pDDCCenterSelected, NULL);
	
	Offscreen.dwWidth = 120;
	Offscreen.dwHeight = 90;
	for (i = 0; i < 66; i++)
	{
		pDD7->CreateSurface(&Offscreen,&pDDSprite120x90[i],NULL);
		pDDSprite120x90[i]->SetColorKey(DDCKEY_SRCBLT,&key);
	}

	ZeroMemory(&Offscreen, sizeof(DDSURFACEDESC2));
	Offscreen.dwSize=sizeof(DDSURFACEDESC2);

	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 640;
	Offscreen.dwHeight = 480;
	pDD7->CreateSurface(&Offscreen,&pDDOffscreen,NULL);
	pDDOffscreen->SetColorKey(DDCKEY_SRCBLT,&key);

	ZeroMemory(&Offscreen, sizeof(DDSURFACEDESC2));
	Offscreen.dwSize=sizeof(DDSURFACEDESC2);
	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 32;
	Offscreen.dwHeight = 32;
	pDD7->CreateSurface(&Offscreen, &pDDCursor, NULL);
	pDDCursor->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.dwWidth = 125;
	Offscreen.dwHeight = 600;
	pDD7->CreateSurface(&Offscreen, &pDDPanel, NULL);
	pDDPanel->SetColorKey(DDCKEY_SRCBLT, &key);
	
	Offscreen.dwWidth = 185;
	Offscreen.dwHeight = 160;
	pDD7->CreateSurface(&Offscreen, &pDDSprite185x160, NULL);
	
	Offscreen.dwWidth = 120;
	Offscreen.dwHeight = 100;
	for (i=0; i<4; i++)
		pDD7->CreateSurface(&Offscreen, &pDDSprite120x100[i], NULL);

	Offscreen.dwWidth = 170;
	Offscreen.dwHeight = 160;
	pDD7->CreateSurface(&Offscreen, &pDDSprite170x160, NULL);

	Offscreen.dwWidth = 80;
	Offscreen.dwHeight = 45;
	pDD7->CreateSurface(&Offscreen, &pDDMenuButtonPressed, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDMenuButtonOver, NULL);

	Offscreen.dwWidth = 80;
	Offscreen.dwHeight = 60;
	pDD7->CreateSurface(&Offscreen, &pDDPlaneButton, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDPlaneButtonPressed, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDF15Button, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDF15ButtonPressed,NULL);

	return TRUE;
}

void DirectDrawUnInit(int iType)
{
	if (iType==UNINIT_GAME)
	{
		RELEASE(pDDF15ButtonPressed);
		RELEASE(pDDF15Button);
		RELEASE(pDDPlaneButtonPressed);
		RELEASE(pDDPlaneButton);
		RELEASE(pDDMenuButtonOver);
		RELEASE(pDDMenuButtonPressed);
		RELEASE(pDDSprite170x160);
		for (int i=3; i>=0; i--)
			RELEASE(pDDSprite120x100[i]);
		RELEASE(pDDSprite185x160);
		RELEASE(pDDPanel);
		RELEASE(pDDCursor);
		RELEASE(pDDOffscreen);
		RELEASE(pDDUnitSelection);
		RELEASE(pDDCCenterSelected);
		for (i=65; i>=0; i--)
				RELEASE(pDDSprite120x90[i]);
		for (i=MAX_TILES-1; i>=0; i--)
			RELEASE(pDDTile[i]);
	}

	if (iType==UNINIT_MENU)
	{
		RELEASE(pDDMenuExitTXTSel);
		RELEASE(pDDMenuOptTXTSel);
		RELEASE(pDDMenuBeginTXTSel);
		RELEASE(pDDMenuBeginTXT);
		RELEASE(pDDMenuOpt);
		RELEASE(pDDMenuBegin);
		RELEASE(pDDCursor);
		RELEASE(pDDOffscreen);
	}

	RELEASE(pDDClipper);
	RELEASE(pDDBackBuffer);
	RELEASE(pDDPrimary);
	RELEASE(pDD7);
}

void DirectInputUnInit()
{
	pDIMouse->Unacquire();
	RELEASE(pDIMouse);
	pDIKeyboard->Unacquire();
	RELEASE(pDIKeyboard);
	RELEASE(pDI);
}

void Loading(int BackgroundType, int Motion)
{
	HDC hdc;
	HRESULT hr;

	switch (Motion)
	{
	case 0:
		pDDOffscreen->GetDC(&hdc);
		Load.Draw(hdc);
		pDDOffscreen->ReleaseDC(hdc);

		if (BackgroundType == LBT_BLACK)
			hr = pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);
		if (BackgroundType == LBT_TRANSPARENT)
			pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		hr = pDDPrimary->Flip(NULL, DDFLIP_WAIT);
		if (hr!=DD_OK) PostQuitMessage(0);
		break;
	case 1:
		break;
	}
}

void PlayFile (LPSTR szFile, HWND hwnd)
{
	HRESULT hr;
	WCHAR wFile[MAX_PATH];
	MultiByteToWideChar( CP_ACP, 0, szFile, -1, wFile, MAX_PATH );

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pigb);

	if (SUCCEEDED(hr))
	{
		pigb->QueryInterface(IID_IMediaControl, (void **)&pimc);
		pigb->QueryInterface(IID_IMediaEventEx, (void **)&pimex);
		pigb->QueryInterface(IID_IVideoWindow, (void **)&pivw);

		hr = pigb->RenderFile(wFile, NULL);
		pivw->put_FullScreenMode(OATRUE);

		// Have the graph signal event via window callbacks for performance
		if (FAILED(pimex->SetNotifyWindow((OAHWND)hwnd, WM_GRAPHNOTIFY, 0)))
			PostQuitMessage(0);
		

		FilmState = PLAYING;

		if (SUCCEEDED(hr))
			pimc->Run();
	}
}

void DarkenScreen()
{
	DDSURFACEDESC2 sDesc;
	sDesc.dwSize=sizeof(sDesc);
	
	if (pDDBackBuffer->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
	{
		PBYTE mem=(PBYTE) sDesc.lpSurface;
		for (int relx=0; relx<800;relx++)
			for (int rely=0; rely<600; rely++)
			{
				int poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;
				USHORT *m=(USHORT *) &mem[poz];
				*m= ((*m)>>1)&0x7BEF;
			}

		pDDBackBuffer->Unlock(NULL);
	}
}

void LoadMenuFiles()
{	
	HDC hdc;
	BOOL b;

	b = MenuBack.Load("data\\Menu\\MainMenu.bmp");
	if (!b) PostQuitMessage(0);

	b = Cursor.Load("data\\Interface\\Cursor.bmp");
		
	HRESULT hr = pDDCursor->GetDC(&hdc);
	if(FAILED(hr)) PostQuitMessage(0);
	b = Cursor.Draw(hdc);
	pDDCursor->ReleaseDC(hdc);

	BGActual = new CBmpList;
	BGFirst = BGActual;
	BGActual->Load("data\\Menu\\BeginGame\\bg0.bmp");
	for (int i = 1; i <=25; i++)
	{
		char buffer[256];
		sprintf(buffer, "data\\Menu\\BeginGame\\bg%d.bmp", i);
		
		BGNext = new CBmpList;
		BGActual->next = BGNext;
		BGActual = BGNext;
		BGActual->Load(buffer);
	}
	BGActual->next = BGFirst;
	BGActual = BGFirst;

	OptActual = new CBmpList;

	OptFirst = OptActual;
	OptActual->Load("data\\Menu\\Options\\opt0.bmp");
	for (i = 1; i <=29; i++)
	{
		char buffer[256];
		sprintf(buffer, "data\\Menu\\Options\\opt%d.bmp", i);
		
		OptNext = new CBmpList;
		OptActual->next = OptNext;
		OptActual = OptNext;
		OptActual->Load(buffer);
	}
	OptActual->next = OptFirst;
	OptActual = OptFirst;

	CBmp bmpBeginText, bmpBeginTextSel, bmpOptTextSel, bmpExitTextSel;
	bmpBeginText.Load("data\\Menu\\startgame.bmp");
	bmpBeginTextSel.Load("data\\Menu\\startgamesel.bmp");
	bmpOptTextSel.Load("data\\Menu\\optionsel.bmp");
	bmpExitTextSel.Load("data\\Menu\\exitsel.bmp");

	pDDMenuBeginTXT->GetDC(&hdc);
	bmpBeginText.Draw(hdc);
	pDDMenuBeginTXT->ReleaseDC(hdc);
	pDDMenuBeginTXTSel->GetDC(&hdc);
	bmpBeginTextSel.Draw(hdc);
	pDDMenuBeginTXTSel->ReleaseDC(hdc);
	pDDMenuOptTXTSel->GetDC(&hdc);
	bmpOptTextSel.Draw(hdc);
	pDDMenuOptTXTSel->ReleaseDC(hdc);
	pDDMenuExitTXTSel->GetDC(&hdc);
	bmpExitTextSel.Draw(hdc);
	pDDMenuExitTXTSel->ReleaseDC(hdc);
}

BOOL StartGame(HWND hwnd)
{
	if (INIT_ERROR) return FALSE;
	if (!DirectInputInit(hwnd))
	{
		MessageBox(hwnd,"Fatal error occured. The game cannot continue.","Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	if (!DirectDrawInit(640,480,hwnd))
	{
		MessageBox(hwnd,"Fatal error occured. The game cannot continue.","Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	CreateMenuOffscreenSurfaces();
	
	BOOL b;
	b = Load.Load("data\\Interface\\Loading.bmp");
	if (!b) PostQuitMessage(0);

	Loading(LBT_BLACK, LM_STATIC);
	LoadMenuFiles();

	//PlayFile("C:\\MP3\\Rock\\Other\\Apocalyptica - Hope.mp3", hwnd);
	//PlayFile("C:\\MP3\\Rock\\Manowar\\1996 Louder than Hell\\LTHELL08.MP3", hwnd);
	while (UpdateGame(hwnd));
	DirectInputUnInit();
	DirectDrawUnInit(UNINIT_GAME);
	return FALSE;
}

BOOL UpdateMainMenu(HWND hwnd)
{
	static Selected = 0, SelOld, contor = 0, c = 0;
	static CurX = 320, CurY = 160, CurXAnte = 320, CurYAnte = 160;
	
	DIMOUSESTATE dims;
	HDC hdc;
	BOOL InRect, MEnter = FALSE, mGo = FALSE;
	int max = 3;

	pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
	pDIKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
	
	CurX+=dims.lX * MouseSensitivity / 100;
	CurY+=dims.lY * MouseSensitivity / 100;

	pDDOffscreen->GetDC(&hdc);
	MenuBack.Draw(hdc);
	SetTextAlign(hdc, TA_BOTTOM | TA_RIGHT);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255,255,255));
	TextOut(hdc, 640, 480, VERSION, strlen(VERSION));
	pDDOffscreen->ReleaseDC(hdc);
	pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);

	POINT pCursor = {CurX, CurY};
	RECT DestRect, rBeginGame, rOptions, rExit;

	SetRect(&rBeginGame, 40, 170, 40 + 230, 170 + 240);
	SetRect(&rOptions, 380, 210, 380+165, 210 + 200);
	SetRect(&rExit, 280, 440, 280 + 80, 440 + 30);

	if (CurX<0) CurX = 0;
	if (CurY<0) CurY = 0;
	if (CurX>640) CurX = 640;
	if (CurY>480) CurY = 480;

	if (CurXAnte<0) CurXAnte = 0;
	if (CurYAnte<0) CurYAnte = 0;
	if (CurXAnte>640) CurXAnte = 640;
	if (CurYAnte>480) CurYAnte = 480;

	InRect = FALSE;
	if ((CurXAnte != CurX) || (CurYAnte != CurY))
	{
		Selected = 0;
		if (PtInRect(&rBeginGame, pCursor)) Selected = 1;
		if (PtInRect(&rOptions, pCursor)) Selected = 2;
		if (PtInRect(&rExit, pCursor)) Selected = 3;
	}
	if (PtInRect(&rBeginGame, pCursor)) InRect = TRUE;
	if (PtInRect(&rOptions, pCursor)) InRect = TRUE;
	if ((Selected == 1)&& (InRect))
	{
		c++;
		if (c % 2 == 0) BGActual = BGActual->next;
	}
	if ((Selected == 2)&& (InRect)) OptActual = OptActual->next;

	pDDMenuBegin->GetDC(&hdc);
	BGActual->Draw(hdc);
	pDDMenuBegin->ReleaseDC(hdc);

	pDDMenuOpt->GetDC(&hdc);
	OptActual->Draw(hdc);
	pDDMenuOpt->ReleaseDC(hdc);

	SetRect(&DestRect, 40, 170, 40 + 220, 170 + 220);
	pDDBackBuffer->Blt(&DestRect, pDDMenuBegin, NULL, DDBLT_WAIT, NULL);
	SetRect(&DestRect, 360, 200, 360 + 170, 200 + 150);
	pDDBackBuffer->Blt(&DestRect, pDDMenuOpt, NULL, DDBLT_WAIT, NULL);
	if (Selected==1)
	{
		SetRect(&DestRect, 40, 365, 40 + 232, 365 + 48);
		pDDBackBuffer->Blt(&DestRect, pDDMenuBeginTXTSel, NULL, DDBLT_WAIT, NULL);
	}
	else
	{
		SetRect(&DestRect, 43, 368, 43 + 227, 368 + 42);
		pDDBackBuffer->Blt(&DestRect, pDDMenuBeginTXT, NULL, DDBLT_WAIT, NULL);
	}
	if (Selected==2)
	{
		SetRect(&DestRect, 380, 365, 380 + 170, 365 + 50);
		pDDBackBuffer->Blt(&DestRect, pDDMenuOptTXTSel, NULL, DDBLT_WAIT, NULL);
	}
	if (Selected==3)
	{
		SetRect(&DestRect, 270, 430, 270 + 100, 430 + 50);
		pDDBackBuffer->Blt(&DestRect, pDDMenuExitTXTSel, NULL, DDBLT_WAIT, NULL);
	}

	SetRect(&DestRect, CurX, CurY, CurX + 32, CurY + 32);
	pDDBackBuffer->Blt(&DestRect, pDDCursor, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

	pDDPrimary->Flip(NULL, DDFLIP_WAIT);

	if (dims.rgbButtons[0] & 0x80) MEnter = TRUE;
	if ((Selected == 1) && MEnter) 
	{
		State = GAME_ACTIVE;
		DirectDrawUnInit(UNINIT_MENU);
		DirectDrawInit(800,600,hwnd);
		CreateGameOffscreenSurfaces();
		Loading(LBT_BLACK, LM_STATIC);
		Engine.Load();
		pDDCursor->GetDC(&hdc);
		Cursor.Draw(hdc);
		pDDCursor->ReleaseDC(hdc);
	}
	if (KEYDOWN(buffer, DIK_ESCAPE)||((Selected == 3) && MEnter)) return FALSE;

	return TRUE;
}

int UpdateGame(HWND hwnd)
{
	switch (State)
	{
		case GAME_ACTIVE:
			if (!Engine.Update()) return 0;
			break;
		case MAIN_MENU:
			if (!UpdateMainMenu(hwnd)) return 0;
			break;
	}
	return 1;
}