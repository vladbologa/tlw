/*----------------------------------------------
  DxGame.cpp - "The Last War" main source file
	(c) 1999-2002 Vlad Bologa & Emil Butiri
------------------------------------------------*/

#include "stdafx.h"
#include "Bmp.h"

#include "map.h"
#include "unitplane.h"
#include "Misile.h"
#include "struccenter.h"
#include "strupyramid.h"
#include "structoilplant.h"
#include "structairport.h"

#include <stdio.h>
#include <streams.h>
#include <string.h>

#include <ddraw.h>
#include <dinput.h>

#define  VERSION "v0.6.1b"
 
#define KEYDOWN(name,key) (name[key] & 0x80)
#define WM_GRAPHNOTIFY  WM_USER+13
#define WM_BEGINGAME	WM_USER+14
#define RELEASE(x) { if (x) x->Release(); x = NULL; }

#define PLAYING			TRUE
#define STOPPED			FALSE

#define TILESIZE		160
#define MAX_TILES		5

#define CCENTER			1
#define OILPLANT		2
#define PYRAMID			3
#define AIRPORT			4

#define LWU_AIR			1	
#define LWU_PLANE		1
#define LWU_F15			2

#define UNIT			0
#define STRUCT			1

#define UNINIT_MENU		1
#define UNINIT_GAME		2
		
enum GameState
{
	MAIN_MENU = 0,
	GAME_ACTIVE,
	GAME_PAUSED,
	NEW_GAME,
	NEW_CUSTOM_GAME,
	NEW_GAME_ERROR,
	MENU_OPTIONS,
	GAME_OPTIONS
};

GameState State = MAIN_MENU;
BOOL FilmState = STOPPED;
BOOL IsReading = FALSE;
BOOL EnterDialog;
int iFileNameLen;

RECT ScreenSize;

CBmp bmp;
CBmp MenuBack, MINewGame[2], MIExit[2], MILoadGame[2], MIOptions[2];
CBmp Options, bmpOK, bmpOKHover, bmpBtn1, bmpBtn2;
CBmp Load, Cursor;

LONG      evCode;
LONG      evParam1;
LONG      evParam2;

int INIT_ERROR=0;
char buffer[256];

int MouseSensitivity = 2;
int Music=3;
int CurX = 320, CurY = 160;
int iResX, iResY;

//DirectX & DirectX Media COM Objects
IBaseFilter   *pif   = NULL;
IGraphBuilder *pigb  = NULL;
IMediaControl *pimc  = NULL;
IMediaEventEx *pimex = NULL;
IVideoWindow  *pivw  = NULL;

IBaseFilter   *pifMusic   = NULL;
IGraphBuilder *pigbMusic  = NULL;
IMediaControl *pimcMusic  = NULL;
IMediaEventEx *pimexMusic = NULL;

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
LPDIRECTDRAWSURFACE7 pDDMenuSM, pDDSMNewGame, pDDSMCustom;
LPDIRECTDRAWSURFACE7 pDDMenuBeginTXT, pDDMenuBeginTXTSel, pDDMenuOptTXTSel, pDDMenuExitTXTSel;
LPDIRECTDRAWSURFACE7 pDDSprite120x90[66];
LPDIRECTDRAWSURFACE7 pDDSpriteOilPlant, pDDSprite185x160, pDDSprite170x160;
LPDIRECTDRAWSURFACE7 pDDCCenterBuild, pDDAirBuild, pDDOilBuild, pDDPyrBuild;
LPDIRECTDRAWSURFACE7 pDDAirport, pDDAirportSelected, pDDAirportMask;
LPDIRECTDRAWSURFACE7 pDDUnitSelection, pDDPyramid, pDDPyramidSelected, pDDPyramidMask;
LPDIRECTDRAWSURFACE7 pDDCCenterSelected, pDDCCenterMask;
LPDIRECTDRAWSURFACE7 pDDOilPlantSelected, pDDOilPlantMask;
LPDIRECTDRAWSURFACE7 pDDMenuButtonPressed, pDDMenuButtonOver, pDDPauseMenu;
LPDIRECTDRAWSURFACE7 pDDPlaneButton, pDDPlaneButtonPressed;
LPDIRECTDRAWSURFACE7 pDDF15Button, pDDF15ButtonPressed;
LPDIRECTDRAWSURFACE7 pDDCCenterButton, pDDCCenterButtonPressed;
LPDIRECTDRAWSURFACE7 pDDOilPlantButton, pDDOilPlantButtonPressed;
LPDIRECTDRAWSURFACE7 pDDAirportButton, pDDAirportButtonPressed;
LPDIRECTDRAWSURFACE7 pDDPyramidButton, pDDPyramidButtonPressed;
LPDIRECTDRAWSURFACE7 pDDGreenFrame, pDDRedFrame;
LPDIRECTDRAWSURFACE7 pDDPauseOptions, pDDPauseEMenu, pDDPauseEGame, pDDPauseCont;
LPDIRECTDRAWSURFACE7 pDDOptions, pDDOptionsOKP, pDDOptionsOKHover, pDDOptionsBtn1, pDDOptionsBtn2;
LPDIRECTDRAWSURFACE7 pDDCustom, pDDCustomOKP, pDDCustomOKHover;
LPDIRECTDRAWSURFACE7 pDDError, pDDErrorOKP, pDDErrorOKHover;
LPDIRECTDRAWSURFACE7 pDDMissile;

//Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL StartGame(HWND);
BOOL DirectDrawInit(int,int,HWND);
BOOL DirectInputInit(HWND);
BOOL TileInScreen(RECT);
BOOL UpdateMainMenu();
BOOL UpdateOptions(int, int);
BOOL UpdateStartCustom(HWND);
BOOL UpdateStartMenu(HWND);
BOOL UpdateNewGameError();
BOOL CreateGameOffscreenSurfaces();
BOOL CreateMenuOffscreenSurfaces();
void DirectDrawUnInit(int);
void DirectInputUnInit();
int UpdateGame(HWND);
void LoadMenuFiles();
void PlayFile(LPSTR, HWND);
void PlayMusic(LPSTR);
void Loading();
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
	BOOL MenuButtonPressed, MenuButtonPressedOld, PlaneButtonPressed, PlaneButtonPressedOld, bF15BtnP, bF15BtnPOld;
	BOOL bCCenterBP, bCCenterBPOld, bOPBP, bOPBPOld, bAirBP, bAirBPOld, bPyBP, bPyBPOld;
	CMap Map;
	int UnitCount, SelectedCount;
	int StructSelType, iCreateBuilding;
	
	CUnit *Unit, *first, *temp;
	CStructure *Struct, *sfirst, *stemp;
	CMissile *Missile, *mfirst, *mtemp;
public:
	BOOL UpdatePauseMenu(HWND hwnd);
	void AddMissile(CUnit *Parent, CUnit *uDest, CStructure *sDest, int iDestType);
	void GetMouseCoords(int &x, int &y);
	void ShowMouse();
	void CorrectCoords();
	BOOL Update(int Reserved = 0);
	BOOL UpdateTerrain(int x = 0, int y = 0);
	BOOL Load(char *filename);
	void LoadTerrainTiles(int tileset = 0);
	void AddUnit(int iType, int iSubType, CStructure *Parent);
};

BOOL GameEngine::Update(int Reserved)
{
	HDC hdc;
	int m_x, m_y, iNrSel=0, iStructSel=0, iTypeAtt=0, create_x, create_y,i,j;
	POINT pCursor;
	RECT rButton,rButtonSm;
	HRESULT hr;
	static HPEN hPen=CreatePen(PS_SOLID, 1, RGB(20,200,40));
	static int add=0;
	BOOL bMouseOnPanel, bAttack=FALSE;

	GetMouseCoords(m_x, m_y);
	pDIKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
	if (!bLeftBtnPressed) 
	{
		MenuButtonPressed=PlaneButtonPressed=bF15BtnP=FALSE;
		bCCenterBP=bOPBP=bAirBP=bPyBP=FALSE;
	}
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
	if (RightButtonPressed) iCreateBuilding=0;
	if (!iCreateBuilding)
	{
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
			if (rc.right>704) rc.right=705;

			if (IntersectRect(&temp, &rc, &r_Unit)) 
			{
				Unit->Select(TRUE);
				iNrSel++;
			}
			else Unit->Select(FALSE);
			Unit=Unit->next;
		}
		
		Struct=sfirst;
		while (Struct)
		{
			pt.x=Struct->GetX()-CurentX;
			pt.y=Struct->GetY()-CurentY;
				
			rc.top=fmouse_y;
			rc.left=fmouse_x;
			rc.bottom=mouse_y;
			rc.right=mouse_x;

			r_Structure.top=pt.y;
			r_Structure.left=pt.x;
			switch (Struct->GetType())
			{
			case 1:
				r_Structure.bottom=pt.y+160;
				r_Structure.right=pt.x+185;
				break;
			case 2:
				r_Structure.bottom=pt.y+212;
				r_Structure.right=pt.x+295;
				break;
			case 3:
				r_Structure.bottom=pt.y+160;
				r_Structure.right=pt.x+180;
				break;
			case 4:
				r_Structure.bottom=pt.y+265;
				r_Structure.right=pt.x+290;
				break;
			}
						
			if (rc.top>rc.bottom){ tmp=rc.top; rc.top=rc.bottom; rc.bottom=tmp;}
			if (rc.left>rc.right){ tmp=rc.left; rc.left=rc.right; rc.right=tmp;}
			if (rc.right>704) rc.right=705;

			if (IntersectRect(&temp, &rc, &r_Structure)&&(!iNrSel))
			{
				Struct->Select(TRUE);
				StructSelType=Struct->GetType();
				iNrSel++;
				iStructSel++;
			}
			else
			{
				Struct->Select(FALSE);
				if (!iStructSel) StructSelType=0;
			}
			Struct=Struct->next;
		}
		
		SelectedCount=iNrSel;
		IsSelecting=FALSE;
		WaitSelection=FALSE;
	}

	if (WaitSelection&&!bLeftBtnPressed)
	{
		BOOL usel = FALSE;
		RECT r_Structure, r_Unit;
		int CursorOnUnit = 0, CursorOnStructure = 0;
		POINT pt;
		
		pt.x=mouse_x;
		pt.y=mouse_y;

		Unit=first;
		if (Unit) while (Unit->next) Unit=Unit->next;
		while (Unit)
		{
			CursorOnUnit = 0;
			r_Unit.top=Unit->GetY()-CurentY;
			r_Unit.left=Unit->GetX()-CurentX;
			r_Unit.bottom=r_Unit.top+90;
			r_Unit.right=r_Unit.left+120;

			if (PtInRect(&r_Unit, pt))
			{
				int relx, rely, poz;
				DDSURFACEDESC2 sDesc;
				sDesc.dwSize=sizeof(sDesc);
				int frame=Unit->GetCurrentFrame();
				
				relx=mouse_x-r_Unit.left;
				rely=mouse_y-r_Unit.top;
				if (Unit->GetSubType()==2) frame+=33;
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
				if (!usel)
				{
					Unit->Select(TRUE);
					usel=TRUE;
					iNrSel++;
				}
				else Unit->Select(FALSE);
			}
			else Unit->Select(FALSE);
			Unit=Unit->prev;
		}
		
		Struct=sfirst;
		while (Struct)
		{
			CursorOnStructure = 0;
			r_Structure.top=Struct->GetY()-CurentY;
			r_Structure.left=Struct->GetX()-CurentX;
			switch (Struct->GetType())
			{
			case 1:
				r_Structure.bottom=r_Structure.top + 160;
				r_Structure.right=r_Structure.left + 185;
				break;
			case 2:
				r_Structure.bottom=r_Structure.top + 212;
				r_Structure.right=r_Structure.left + 295;
				break;
			case 3:
				r_Structure.bottom=r_Structure.top + 160;
				r_Structure.right=r_Structure.left + 180;
				break;
			case 4:
				r_Structure.bottom=r_Structure.top + 265;
				r_Structure.right=r_Structure.left + 290;
				break;
			}

			if (PtInRect(&r_Structure, pt))
			{
				int relx, rely, poz;
				DDSURFACEDESC2 sDesc;
				sDesc.dwSize=sizeof(sDesc);	
					
				relx=mouse_x-r_Structure.left;
				rely=mouse_y-r_Structure.top;
				switch (Struct->GetType())
				{
				case 1:
					if (pDDCCenterMask->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
					{
						PBYTE mem=(PBYTE) sDesc.lpSurface;
						poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

						if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnStructure=1;
							pDDCCenterMask->Unlock(NULL);
					}
					break;
				case 2:
					if (pDDOilPlantMask->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
					{
						PBYTE mem=(PBYTE) sDesc.lpSurface;
						poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

						if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnStructure=1;
							pDDOilPlantMask->Unlock(NULL);
					}
					break;
				case 3:
					if (pDDPyramidMask->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
					{
						PBYTE mem=(PBYTE) sDesc.lpSurface;
						poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

						if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnStructure=1;
							pDDPyramidMask->Unlock(NULL);
					}
					break;
				case 4:
					if (pDDAirportMask->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
					{
						PBYTE mem=(PBYTE) sDesc.lpSurface;
						poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

						if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnStructure=1;
							pDDAirportMask->Unlock(NULL);
					}
					break;
				}
			}

			if (CursorOnStructure&&(!iNrSel))
			{
				Struct->Select(TRUE);
				StructSelType=Struct->GetType();
				iNrSel++;
				iStructSel++;
			}
			else
			{
				Struct->Select(FALSE);
				if (!iStructSel) StructSelType=0;
			}
			Struct=Struct->next;
		}

		SelectedCount=iNrSel;
		WaitSelection=FALSE;
	}

	if (bLeftBtnPressed&&oldLMBPressed&&(!fLMBPressed)&&!IsSelecting&&!bMouseOnPanel) WaitSelection=TRUE;
	if (WaitSelection) if (mouse_x!=fmouse_x&&mouse_y!=fmouse_y) 
	{
		IsSelecting=TRUE;
		WaitSelection=FALSE;
	}
	}
//***********End of Selection***********

	RECT DestRect;

	Struct=sfirst;
	while (Struct)
	{
		switch(Struct->GetType())
		{
		case 1:
			SetRect(&DestRect, Struct->GetX()-CurentX, Struct->GetY()-CurentY, Struct->GetX()+185-CurentX, Struct->GetY()+160-CurentY);
			if (Struct->Selected()) pDDBackBuffer->Blt(&DestRect, pDDCCenterSelected, NULL, DDBLT_WAIT, NULL);
			else pDDBackBuffer->Blt(&DestRect, pDDSprite185x160, NULL, DDBLT_WAIT, NULL);
			break;
		case 2:
			SetRect(&DestRect, Struct->GetX()-CurentX, Struct->GetY()-CurentY, Struct->GetX()+295-CurentX, Struct->GetY()+212-CurentY);
			if (Struct->Selected()) pDDBackBuffer->Blt(&DestRect, pDDOilPlantSelected, NULL, DDBLT_WAIT, NULL);
			else pDDBackBuffer->Blt(&DestRect, pDDSpriteOilPlant, NULL, DDBLT_WAIT, NULL);
			break;
		case 3:
			SetRect(&DestRect, Struct->GetX()-CurentX, Struct->GetY()-CurentY, Struct->GetX()+180-CurentX, Struct->GetY()+160-CurentY);
			if (Struct->Selected()) pDDBackBuffer->Blt(&DestRect, pDDPyramidSelected, NULL, DDBLT_WAIT, NULL);
			else pDDBackBuffer->Blt(&DestRect, pDDPyramid, NULL, DDBLT_WAIT, NULL);
			break;
		case 4:
			SetRect(&DestRect, Struct->GetX()-CurentX, Struct->GetY()-CurentY, Struct->GetX()+290-CurentX, Struct->GetY()+265-CurentY);
			if (Struct->Selected()) pDDBackBuffer->Blt(&DestRect, pDDAirportSelected, NULL, DDBLT_WAIT, NULL);
			else pDDBackBuffer->Blt(&DestRect, pDDAirport, NULL, DDBLT_WAIT, NULL);
			break;
		}
		Struct=Struct->next;
	}

	if (!RightButtonPressed&&oldRMBPressed&&!bMouseOnPanel) 
	{
		BOOL bUSel = FALSE;
		RECT r_Structure, r_Unit;
		int CursorOnUnit = 0, CursorOnStructure = 0;
		int iNrAt=0, iStrAt=0;
		POINT pt;
		
		pt.x=mouse_x;
		pt.y=mouse_y;

		Unit=first;
		while (Unit)
		{
			Unit->Attack(FALSE);
			Unit=Unit->next;
		}
		Struct=sfirst;
		while (Struct)
		{
			Struct->Attack(FALSE);
			Struct=Struct->next;
		}

		Unit=first;
		if (Unit) while (Unit->next) Unit=Unit->next;
		while (Unit)
		{
			CursorOnUnit = 0;
			r_Unit.top=Unit->GetY()-CurentY;
			r_Unit.left=Unit->GetX()-CurentX;
			r_Unit.bottom=r_Unit.top+90;
			r_Unit.right=r_Unit.left+120;

			if (PtInRect(&r_Unit, pt))
			{
				int relx, rely, poz;
				DDSURFACEDESC2 sDesc;
				sDesc.dwSize=sizeof(sDesc);
				int frame=Unit->GetCurrentFrame();
				
				relx=mouse_x-r_Unit.left;
				rely=mouse_y-r_Unit.top;
				if (Unit->GetSubType()==2) frame+=33;
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
				if (!bUSel)
				{
					Unit->Attack(TRUE);
					bAttack=TRUE;
					bUSel=TRUE;
					iNrAt++;
					iTypeAtt=1;
				}
				else Unit->Attack(FALSE);
			}
			else Unit->Attack(FALSE);
			Unit=Unit->prev;
		}
		
		Struct=sfirst;
		while (Struct)
		{
			CursorOnStructure = 0;
			r_Structure.top=Struct->GetY()-CurentY;
			r_Structure.left=Struct->GetX()-CurentX;
			switch (Struct->GetType())
			{
			case 1:
				r_Structure.bottom=r_Structure.top + 160;
				r_Structure.right=r_Structure.left + 185;
				break;
			case 2:
				r_Structure.bottom=r_Structure.top + 212;
				r_Structure.right=r_Structure.left + 295;
				break;
			case 3:
				r_Structure.bottom=r_Structure.top + 160;
				r_Structure.right=r_Structure.left + 180;
				break;
			case 4:
				r_Structure.bottom=r_Structure.top + 265;
				r_Structure.right=r_Structure.left + 290;
				break;
			}

			if (PtInRect(&r_Structure, pt))
			{
				int relx, rely, poz;
				DDSURFACEDESC2 sDesc;
				sDesc.dwSize=sizeof(sDesc);	
					
				relx=mouse_x-r_Structure.left;
				rely=mouse_y-r_Structure.top;
				switch (Struct->GetType())
				{
				case 1:
					if (pDDCCenterMask->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
					{
						PBYTE mem=(PBYTE) sDesc.lpSurface;
						poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

						if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnStructure=1;
							pDDCCenterMask->Unlock(NULL);
					}
					break;
				case 2:
					if (pDDOilPlantMask->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
					{
						PBYTE mem=(PBYTE) sDesc.lpSurface;
						poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

						if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnStructure=1;
							pDDOilPlantMask->Unlock(NULL);
					}
					break;
				case 3:
					if (pDDPyramidMask->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
					{
						PBYTE mem=(PBYTE) sDesc.lpSurface;
						poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

						if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnStructure=1;
							pDDPyramidMask->Unlock(NULL);
					}
					break;
				case 4:
					if (pDDAirportMask->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
					{
						PBYTE mem=(PBYTE) sDesc.lpSurface;
						poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;

						if (mem[poz]!=0&&mem[poz+1]!=0) CursorOnStructure=1;
							pDDAirportMask->Unlock(NULL);
					}
					break;
				}
			}

			if (CursorOnStructure&&(!iNrAt))
			{
				Struct->Attack(TRUE);
				bAttack=TRUE;
				iNrAt++;
				iStrAt++;
				iTypeAtt=2;
			}
			else Struct->Attack(FALSE);
			Struct=Struct->next;
		}
	}

	if (iTypeAtt==1)
	{
		Unit=first;
		while (Unit)
		{
			if (Unit->Attacked()) break;
			Unit=Unit->next;
		}
		temp=first;
		while (temp)
		{
			if (temp->Selected()) AddMissile(temp, Unit, NULL, UNIT);
			temp=temp->next;
		}
	}
	else if (iTypeAtt==2)
	{
		Struct=sfirst;
		while (Struct)
		{
			if (Struct->Attacked()) break;
			Struct=Struct->next;
		}
		temp=first;
		while (temp)
		{
			if (temp->Selected()) AddMissile(temp, NULL, Struct, STRUCT);
			temp=temp->next;
		}
	}

	Missile=mfirst;
	while (Missile)
	{
		Missile->Update();
		SetRect(&DestRect, Missile->GetX()-CurentX, Missile->GetY()-CurentY,40+Missile->GetX()-CurentX, 40+Missile->GetY()-CurentY) ;
		pDDBackBuffer->Blt(&DestRect, pDDMissile, NULL, DDBLT_WAIT|DDBLT_KEYSRC, NULL);
		Missile=Missile->next;
	}

	if (iCreateBuilding)
	{
		int it,jt,bCB=1;
		create_x=mouse_x-(CurentX+mouse_x)%80;
		create_y=mouse_y-(CurentY+mouse_y)%80;
	
		switch (iCreateBuilding)
		{
		case 1:
			SetRect(&DestRect,create_x, create_y, create_x+185, create_y+160);
			pDDBackBuffer->Blt(&DestRect, pDDCCenterBuild, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
			it=3; jt=2;
			break;
		case 2:
			SetRect(&DestRect,create_x, create_y, create_x+295, create_y+212);
			pDDBackBuffer->Blt(&DestRect, pDDOilBuild, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
			it=4; jt=3;
			break;
		case 3:
			SetRect(&DestRect,create_x, create_y, create_x+290, create_y+265);
			pDDBackBuffer->Blt(&DestRect, pDDAirBuild, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
			it=4; jt=4;
			break;
		case 4:
			SetRect(&DestRect,create_x, create_y, create_x+180, create_y+160);
			pDDBackBuffer->Blt(&DestRect, pDDPyrBuild, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
			it=3; jt=2;
			break;
		}
		for (i=0; i<it; i++)
			for (j=0; j<jt; j++)
			{
				SetRect(&DestRect, create_x+80*i, create_y+80*j, create_x+80*(i+1), create_y+80*(j+1));
				if (Map.Available((create_y+80*j+CurentY)/80,(create_x+80*i+CurentX)/80))
					pDDBackBuffer->Blt(&DestRect, pDDGreenFrame, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
				else
				{
					pDDBackBuffer->Blt(&DestRect, pDDRedFrame, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
					bCB=0;
				}
			}
		if (oldLMBPressed&&(!bLeftBtnPressed)&&bCB)
		{
			Struct=sfirst;
			while (Struct->next) Struct=Struct->next;
			switch (iCreateBuilding)
			{
			case 1:
				stemp=(CStructCCenter *) new CStructCCenter;
				break;
			case 2:
				stemp=(CStructOilPlant *) new CStructOilPlant;
				break;
			case 3:
				stemp=(CStructAirport *) new CStructAirport;
				break;
			case 4:
				stemp=(CStructPyramid *) new CStructPyramid;
				break;
			}
			Struct->next=stemp;
			stemp->SetPosition(create_x+CurentX,create_y+CurentY,&Map);
			stemp->next=NULL;
			iCreateBuilding=0;
		}
	}

	int vx,vy,c=0,row=1,p;
	int tx = 0,ty = 0;

	Unit=first;
	while (Unit)
	{
		if (!RightButtonPressed&&oldRMBPressed&&Unit->Selected()&&!bMouseOnPanel) 
		{
			tx+=(CurentX+m_x-60-Unit->GetX());
			ty+=(CurentY+m_y-45-Unit->GetY());
		}
		Unit=Unit->next;
	}

	Unit=first;
	while (Unit)
	{
		if (!RightButtonPressed&&oldRMBPressed&&Unit->Selected()&&!bMouseOnPanel&&!bAttack) 
		{
			c++;
			if (SelectedCount%2)
			{
				if (c<=SelectedCount/2)
				{
					p=((SelectedCount-1)/2)-c+1;
					vx=(-20)*p;
					vy=(-20)*p;
				}
				else if (c>(SelectedCount/2)+1)
				{
					p=c-((SelectedCount-1)/2)-1;
					vx=(-20)*p;
					vy=20*p;
				}
				else vx=vy=0;
			}
			else
			{
				if (c<SelectedCount/2)
				{
					p=((SelectedCount-1)/2)-c+1;
					vx=(-20)*p;
					vy=(-20)*p;
				}
				else if (c>(SelectedCount/2)+1)
				{
					p=c-((SelectedCount-1)/2)-1;
					vx=(-20)*(p-1);
					vy=20*p;
				}
				else
				{
					if (c==SelectedCount/2) vx=vy=0;
					else
					{
						vx=0;
						vy=20;
					}
				}

			}

			if ((tx>0)&&(ty>0))
			{
				if (tx>ty) Unit->SetDestination(CurentX+m_x-60+vx, CurentY+m_y-45+vy);
				else Unit->SetDestination(CurentX+m_x-60+vy, CurentY+m_y-45+vx);
			}
			else if ((tx<0)&&(ty>0))
			{
				if (ty>-tx) Unit->SetDestination(CurentX+m_x-60+vy, CurentY+m_y-45+vx);
				else Unit->SetDestination(CurentX+m_x-60-vx, CurentY+m_y-45+vy);
			}
			else if ((tx<0)&&(ty<0))
			{
				if (tx<ty) Unit->SetDestination(CurentX+m_x-60-vx, CurentY+m_y-45+vy);
				else Unit->SetDestination(CurentX+m_x-60+vy, CurentY+m_y-45-vx);
			}
			else if ((tx>0)&&(ty<0))
			{
				if (-ty>tx) Unit->SetDestination(CurentX+m_x-60+vy, CurentY+m_y-45-vx);
				else Unit->SetDestination(CurentX+m_x-60+vx, CurentY+m_y-45+vy);
			}
		}
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

	if (StructSelType==AIRPORT)
	{
		//Add Plane Button
		SetRect(&rButton,715,130,715+80,130+60);
		SetRect(&rButtonSm,715,130,715+80,130+54);
		if (PtInRect(&rButtonSm, pCursor))
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
				Struct=sfirst;
				while (!Struct->Selected()) Struct=Struct->next;
				AddUnit(LWU_AIR, LWU_PLANE, Struct);
				add=1;
			}
		}
		else add=0;

		//Add F15 Button
		SetRect(&rButton,715,185,715+80,185+60);
		SetRect(&rButtonSm,715,185,715+80,185+54);
		if (PtInRect(&rButtonSm, pCursor))
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
				Struct=sfirst;
				while (!Struct->Selected()) Struct=Struct->next;
				AddUnit(LWU_AIR, LWU_F15, Struct);
				add=1;
			}
		}
		else add=0;
	}
	else if (StructSelType==CCENTER)
	{
		//Add Command Center Button
		SetRect(&rButton,715,130,715+80,130+60);
		SetRect(&rButtonSm,715,130,715+80,130+54);
		if (PtInRect(&rButtonSm, pCursor))
		{
			if (bLeftBtnPressed)
			{
				if (!oldLMBPressed) bCCenterBP=TRUE;
				if (bCCenterBP) pDDBackBuffer->Blt(&rButton, pDDCCenterButtonPressed, NULL, DDBLT_WAIT, NULL);
				else pDDBackBuffer->Blt(&rButton, pDDCCenterButton, NULL, DDBLT_WAIT, NULL);
			}
			else pDDBackBuffer->Blt(&rButton, pDDCCenterButton, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rButton, pDDCCenterButton, NULL, DDBLT_WAIT, NULL);
		if (PtInRect(&rButton,pCursor)&&bCCenterBPOld&&(!bLeftBtnPressed)) iCreateBuilding=1;

		//Add OilPlant Button
		SetRect(&rButton,715,185,715+80,185+60);
		SetRect(&rButtonSm,715,185,715+80,185+54);
		if (PtInRect(&rButtonSm, pCursor))
		{
			if (bLeftBtnPressed)
			{
				if (!oldLMBPressed) bOPBP=TRUE;
				if (bOPBP) pDDBackBuffer->Blt(&rButton, pDDOilPlantButtonPressed, NULL, DDBLT_WAIT, NULL);
				else pDDBackBuffer->Blt(&rButton, pDDOilPlantButton, NULL, DDBLT_WAIT, NULL);
			}
			else pDDBackBuffer->Blt(&rButton, pDDOilPlantButton, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rButton, pDDOilPlantButton, NULL, DDBLT_WAIT, NULL);
		if (PtInRect(&rButton,pCursor)&&bOPBPOld&&(!bLeftBtnPressed)) iCreateBuilding=2;

		//Add Airport Button
		SetRect(&rButton,715,240,715+80,240+60);
		SetRect(&rButtonSm,715,240,715+80,240+54);
		if (PtInRect(&rButtonSm, pCursor))
		{
			if (bLeftBtnPressed)
			{
				if (!oldLMBPressed) bAirBP=TRUE;
				if (bAirBP) pDDBackBuffer->Blt(&rButton, pDDAirportButtonPressed, NULL, DDBLT_WAIT, NULL);
				else pDDBackBuffer->Blt(&rButton, pDDAirportButton, NULL, DDBLT_WAIT, NULL);
			}
			else pDDBackBuffer->Blt(&rButton, pDDAirportButton, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rButton, pDDAirportButton, NULL, DDBLT_WAIT, NULL);
		if (PtInRect(&rButton,pCursor)&&bAirBPOld&&(!bLeftBtnPressed)) iCreateBuilding=3;

		//Add Pyramid Button
		SetRect(&rButton,715,295,715+80,295+60);
		SetRect(&rButtonSm,715,295,715+80,295+54);
		if (PtInRect(&rButtonSm, pCursor))
		{
			if (bLeftBtnPressed)
			{
				if (!oldLMBPressed) bPyBP=TRUE;
				if (bPyBP) pDDBackBuffer->Blt(&rButton, pDDPyramidButtonPressed, NULL, DDBLT_WAIT, NULL);
				else pDDBackBuffer->Blt(&rButton, pDDPyramidButton, NULL, DDBLT_WAIT, NULL);
			}
			else pDDBackBuffer->Blt(&rButton, pDDPyramidButton, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rButton, pDDPyramidButton, NULL, DDBLT_WAIT, NULL);
		if (PtInRect(&rButton,pCursor)&&bPyBPOld&&(!bLeftBtnPressed)) iCreateBuilding=4;

	}
		
	/*pDDBackBuffer->GetDC(&hdc);
	SetTextAlign(hdc, TA_BOTTOM | TA_RIGHT);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255,255,255));
	char _itoa_t[10];
	_itoa(SelectedCount, _itoa_t,10);
	TextOut(hdc, 800, 600, _itoa_t, strlen(_itoa_t));
	pDDBackBuffer->ReleaseDC(hdc);*/

	SetRect(&rButton,710,540,710 + 80,540 + 45);
	if (PtInRect(&rButton,pCursor)&&MenuButtonPressedOld&&(!bLeftBtnPressed))
	{
		State=GAME_PAUSED;
		pDDOffscreen->Blt(NULL, pDDBackBuffer, NULL, DDBLT_WAIT, NULL);
		DarkenScreen();
	}
	
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
	MenuButtonPressedOld=MenuButtonPressed;
	bF15BtnPOld=bF15BtnP;
	bCCenterBPOld=bCCenterBP;
	bOPBPOld=bOPBP;
	bAirBPOld=bAirBP;
	bPyBPOld=bPyBP;

	pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	
	int change=0;
	if (KEYDOWN(buffer, DIK_DELETE))
	{
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

	do
	{
		Missile=mfirst;
		change=0;
		if (mfirst)
		{
			if (mfirst->Destroyed())
			{
				mtemp=mfirst->next;
				delete mfirst;
				mfirst=mtemp;
				change=1;
			}
			else while (Missile->next)
			{
				if (Missile->next->Destroyed())
				{
					mtemp=Missile->next->next;
					delete Missile->next;
					Missile->next=mtemp;
					change=1;
					break;
				}
				Missile=Missile->next;
			}
		}
	}
	while (change);
	return TRUE;
}

BOOL GameEngine::UpdatePauseMenu(HWND hwnd)
{
	HDC hdc;
	RECT rMenu, rOptions, rEMenu, rEGame, rCont;
	POINT pCursor;
	int m_x, m_y, Selected;
	
	Selected=0;
	GetMouseCoords(m_x, m_y);
	pCursor.x=m_x;
	pCursor.y=m_y;
	pDIKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
	if (KEYDOWN(buffer, DIK_ESCAPE)) State=GAME_ACTIVE;
	SetRect(&rMenu, 259,119,541,480);
	SetRect(&rOptions, 317,211,317+165,211+45);
	SetRect(&rEMenu,270,270,270+260,270+70);
	SetRect(&rEGame,302,342,302+195,342+40);
	SetRect(&rCont, 305,415,305+185,415+45);
	pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);
	pDDBackBuffer->Blt(&rMenu, pDDPauseMenu, NULL, DDBLT_WAIT, NULL);
	
	if (PtInRect(&rOptions, pCursor)) Selected = 1;
	if (PtInRect(&rEMenu, pCursor)) Selected = 2;
	if (PtInRect(&rEGame, pCursor)) Selected = 3;
	if (PtInRect(&rCont, pCursor)) Selected = 4;

	switch (Selected)
	{
	case 1:
		pDDBackBuffer->Blt(&rOptions, pDDPauseOptions, NULL, DDBLT_WAIT, NULL);
		if (bLeftBtnPressed)
		{
			pDDOffscreen->Blt(NULL, pDDBackBuffer, NULL, DDBLT_WAIT, NULL);
			CurX=m_x;
			CurY=m_y;
			State=GAME_OPTIONS;
		}
		break;
	case 2:
		pDDBackBuffer->Blt(&rEMenu, pDDPauseEMenu, NULL, DDBLT_WAIT, NULL);
		if (bLeftBtnPressed)
		{
			if (first)
			{
				Unit=first;
				while (Unit->next)
				{
					temp=Unit;
					Unit=Unit->next;
					delete temp;
				}
				delete Unit;
				first=NULL;
			}
			if (mfirst)
			{
				Missile=mfirst;
				while (Missile->next)
				{
					mtemp=Missile;
					Missile=Missile->next;
					delete mtemp;
				}
				delete Missile;
				mfirst=NULL;
			}
			if (sfirst)
			{
				Struct=sfirst;
				while (Struct->next)
				{
					stemp=Struct;
					Struct=Struct->next;
					delete stemp;
				}
				delete Struct;
				sfirst=NULL;
			}

			State=MAIN_MENU;
			DirectDrawUnInit(UNINIT_GAME);
			DirectDrawInit(640,480,hwnd);
			CreateMenuOffscreenSurfaces();
			Loading();
			LoadMenuFiles();
			pDDCursor->GetDC(&hdc);
			Cursor.Draw(hdc);
			pDDCursor->ReleaseDC(hdc);
		}
		break;
	case 3:
		pDDBackBuffer->Blt(&rEGame, pDDPauseEGame, NULL, DDBLT_WAIT, NULL);
		if (bLeftBtnPressed) return FALSE;
		break;
	case 4:
		pDDBackBuffer->Blt(&rCont, pDDPauseCont, NULL, DDBLT_WAIT, NULL);
		if (bLeftBtnPressed) State=GAME_ACTIVE;
		break;
	}
	ShowMouse();
	pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	return TRUE;
}

void GameEngine::AddUnit(int iType, int iSubType, CStructure *Parent)
{
	if (!first)
	{
		first=(CUnitPlane *) new CUnitPlane;
		first->SetParent(Parent);
		first->SetSubType(iSubType);
		first->Select(FALSE);
		first->next=NULL;
		first->prev=NULL;
	}
	else
	{
		temp=(CUnitPlane *) new CUnitPlane;
		temp->SetParent(Parent);
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

void GameEngine::AddMissile(CUnit *Parent, CUnit *uDest, CStructure *sDest, int iDestType)
{
	if (!mfirst)
	{	mfirst=(CMissile *) new CMissile;
	
		mfirst->SetPosition(Parent->GetX(), Parent->GetY());
		if (iDestType==UNIT)
			((CMissile *)mfirst)->SetTarget(uDest, NULL, 1);
		else if (iDestType==STRUCT)
			((CMissile *)mfirst)->SetTarget(NULL, sDest,2);
		mfirst->next=NULL;
	}
	else
	{
		mtemp=(CMissile *) new CMissile;
		mtemp->SetPosition(Parent->GetX(), Parent->GetY());
		if (iDestType==UNIT)
			((CMissile *)mtemp)->SetTarget(uDest, NULL, 1);
		else if (iDestType==STRUCT)
			((CMissile *)mtemp)->SetTarget(NULL, sDest,2);
		mtemp->next=mfirst;
		mfirst=mtemp;
	}
}

BOOL GameEngine::UpdateTerrain(int x, int y)
{
	for (int i = 0; i < Map.GetSizeY(); i++)
		for (int j = 0; j < Map.GetSizeX(); j++)
		{
			RECT DestRect;
			SetRect(&DestRect, (TILESIZE*j)-x, (TILESIZE*i)-y, (TILESIZE*j+TILESIZE)-x, (TILESIZE*i+TILESIZE)-y);
			if (TileInScreen(DestRect))
				pDDBackBuffer->Blt(&DestRect, pDDTile[Map.GetTerrainType(i,j)-1], NULL, DDBLT_WAIT, NULL);
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
	TerrainType[2].Load("data\\Tiles\\tile03.bmp");
	TerrainType[3].Load("data\\Tiles\\tile04.bmp");
	TerrainType[4].Load("data\\Tiles\\tile05.bmp");
	
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
	
	m_x+=dims.lX * MouseSensitivity;
	m_y+=dims.lY * MouseSensitivity;

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

GameEngine::Load(char *filename)
{
	CBmp bmp;
	HDC hdc;
	int i;
	
	UnitCount = CurentX = CurentY = StructSelType = iCreateBuilding = 0;
	IsSelecting = WaitSelection = FALSE;
	fLMBPressed = oldLMBPressed = oldRMBPressed = FALSE;
	MenuButtonPressed = MenuButtonPressedOld=FALSE;
	PlaneButtonPressed = PlaneButtonPressedOld = bF15BtnP = bF15BtnPOld = FALSE;
	bCCenterBP = bCCenterBPOld = bOPBP = bOPBPOld = bAirBP = bAirBPOld = bPyBP = bPyBPOld = FALSE;
	first=NULL;
	mfirst=NULL;

	Map.Load(filename);
	LoadTerrainTiles();	
	
	Struct=(CStructCCenter *) new CStructCCenter;
	Struct->SetPosition(160,160,&Map);
	Struct->prev=NULL;
	sfirst=Struct;
	sfirst->next=NULL;

	pDDOptions->GetDC(&hdc);
	Options.Draw(hdc);
	pDDOptions->ReleaseDC(hdc);

	pDDOptionsOKP->GetDC(&hdc);
	bmpOK.Draw(hdc);
	pDDOptionsOKP->ReleaseDC(hdc);

	pDDOptionsOKHover->GetDC(&hdc);
	bmpOKHover.Draw(hdc);
	pDDOptionsOKHover->ReleaseDC(hdc);

	pDDOptionsBtn1->GetDC(&hdc);
	bmpBtn1.Draw(hdc);
	pDDOptionsBtn1->ReleaseDC(hdc);

	pDDOptionsBtn2->GetDC(&hdc);
	bmpBtn2.Draw(hdc);
	pDDOptionsBtn2->ReleaseDC(hdc);
		
	bmp.Load("data\\interface\\mb01.bmp");
	pDDMenuButtonPressed->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDMenuButtonPressed->ReleaseDC(hdc);

	bmp.Load("data\\interface\\mb02.bmp");
	pDDMenuButtonOver->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDMenuButtonOver->ReleaseDC(hdc);

	bmp.Load("data\\interface\\pb01.bmp");
	pDDPlaneButton->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPlaneButton->ReleaseDC(hdc);

	bmp.Load("data\\interface\\pb02.bmp");
	pDDPlaneButtonPressed->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPlaneButtonPressed->ReleaseDC(hdc);

	bmp.Load("data\\interface\\pb01.bmp");
	pDDPlaneButton->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPlaneButton->ReleaseDC(hdc);

	bmp.Load("data\\interface\\cb01.bmp");
	pDDCCenterButton->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDCCenterButton->ReleaseDC(hdc);

	bmp.Load("data\\interface\\cb02.bmp");
	pDDCCenterButtonPressed->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDCCenterButtonPressed->ReleaseDC(hdc);

	bmp.Load("data\\interface\\ob01.bmp");
	pDDOilPlantButton->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDOilPlantButton->ReleaseDC(hdc);

	bmp.Load("data\\interface\\ob02.bmp");
	pDDOilPlantButtonPressed->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDOilPlantButtonPressed->ReleaseDC(hdc);

	bmp.Load("data\\interface\\ab01.bmp");
	pDDAirportButton->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDAirportButton->ReleaseDC(hdc);

	bmp.Load("data\\interface\\ab02.bmp");
	pDDAirportButtonPressed->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDAirportButtonPressed->ReleaseDC(hdc);

	bmp.Load("data\\interface\\yb01.bmp");
	pDDPyramidButton->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPyramidButton->ReleaseDC(hdc);

	bmp.Load("data\\interface\\yb02.bmp");
	pDDPyramidButtonPressed->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPyramidButtonPressed->ReleaseDC(hdc);

	bmp.Load("data\\interface\\fb01.bmp");
	pDDF15Button->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDF15Button->ReleaseDC(hdc);

	bmp.Load("data\\interface\\fb02.bmp");
	pDDF15ButtonPressed->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDF15ButtonPressed->ReleaseDC(hdc);

	bmp.Load("data\\interface\\pb02.bmp");
	pDDPlaneButtonPressed->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPlaneButtonPressed->ReleaseDC(hdc);

	bmp.Load("data\\structures\\ccnb.bmp");
	pDDCCenterBuild->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDCCenterBuild->ReleaseDC(hdc);

	bmp.Load("data\\Interface\\uselect.bmp");
	pDDUnitSelection->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDUnitSelection->ReleaseDC(hdc);

	bmp.Load("data\\Structures\\ccentersel.bmp");
	pDDCCenterSelected->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDCCenterSelected->ReleaseDC(hdc);

	bmp.Load("data\\Structures\\pyramid.bmp");
	pDDSprite170x160->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDSprite170x160->ReleaseDC(hdc);
	
	bmp.Load("data\\Interface\\lpanel.bmp");
	pDDPanel->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPanel->ReleaseDC(hdc);

	bmp.Load("data\\structures\\CCenter.bmp");
	pDDSprite185x160->GetDC(&hdc);
	if (FAILED(bmp.Draw(hdc))) return FALSE;
	pDDSprite185x160->ReleaseDC(hdc);

	bmp.Load("data\\structures\\CCenterCont.bmp");
	pDDCCenterMask->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDCCenterMask->ReleaseDC(hdc);

	bmp.Load("data\\structures\\oilplant.bmp");
	pDDSpriteOilPlant->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDSpriteOilPlant->ReleaseDC(hdc);

	bmp.Load("data\\structures\\oilplantsel.bmp");
	pDDOilPlantSelected->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDOilPlantSelected->ReleaseDC(hdc);

	bmp.Load("data\\structures\\oilplantcont.bmp");
	pDDOilPlantMask->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDOilPlantMask->ReleaseDC(hdc);

	bmp.Load("data\\structures\\pyramid.bmp");
	pDDPyramid->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPyramid->ReleaseDC(hdc);

	bmp.Load("data\\structures\\pyramidsel.bmp");
	pDDPyramidSelected->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPyramidSelected->ReleaseDC(hdc);

	bmp.Load("data\\structures\\pyramidcont.bmp");
	pDDPyramidMask->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPyramidMask->ReleaseDC(hdc);

	bmp.Load("data\\structures\\airport.bmp");
	pDDAirport->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDAirport->ReleaseDC(hdc);

	bmp.Load("data\\structures\\airportsel.bmp");
	pDDAirportSelected->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDAirportSelected->ReleaseDC(hdc);

	bmp.Load("data\\structures\\airportcont.bmp");
	pDDAirportMask->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDAirportMask->ReleaseDC(hdc);

	bmp.Load("data\\structures\\frame1.bmp");
	pDDGreenFrame->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDGreenFrame->ReleaseDC(hdc);
	
	bmp.Load("data\\structures\\frame2.bmp");
	pDDRedFrame->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDRedFrame->ReleaseDC(hdc);

	bmp.Load("data\\units\\Missile.bmp");
	pDDMissile->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDMissile->ReleaseDC(hdc);

	bmp.Load("data\\structures\\oilnb.bmp");
	pDDOilBuild->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDOilBuild->ReleaseDC(hdc);

	bmp.Load("data\\structures\\airnb.bmp");
	pDDAirBuild->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDAirBuild->ReleaseDC(hdc);

	bmp.Load("data\\structures\\pyrnb.bmp");
	pDDPyrBuild->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPyrBuild->ReleaseDC(hdc);

	bmp.Load("data\\menu\\pausemenu.bmp");
	pDDPauseMenu->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPauseMenu->ReleaseDC(hdc);

	bmp.Load("data\\menu\\optpause.bmp");
	pDDPauseOptions->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPauseOptions->ReleaseDC(hdc);

	bmp.Load("data\\menu\\emenupause.bmp");
	pDDPauseEMenu->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPauseEMenu->ReleaseDC(hdc);
	
	bmp.Load("data\\menu\\egamepause.bmp");
	pDDPauseEGame->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPauseEGame->ReleaseDC(hdc);
	
	bmp.Load("data\\menu\\contpause.bmp");
	pDDPauseCont->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDPauseCont->ReleaseDC(hdc);

	for (i=0; i<=32; i++)
	{
		char buffer[256];
		sprintf(buffer, "data\\Units\\Plane\\plane%d.bmp", i);
		bmp.Load(buffer);

		pDDSprite120x90[i]->GetDC(&hdc);
		if (FAILED(bmp.Draw(hdc))) return FALSE;
		pDDSprite120x90[i]->ReleaseDC(hdc);
	}

	for (i=33; i<=65; i++)
	{
		char buffer[256];
		sprintf(buffer, "data\\Units\\F15\\plane%d.bmp", i-33);
		bmp.Load(buffer);

		pDDSprite120x90[i]->GetDC(&hdc);
		if (FAILED(bmp.Draw(hdc))) return FALSE;
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

	Offscreen.dwWidth = 123;
	Offscreen.dwHeight = 101;
	pDD7->CreateSurface(&Offscreen, &pDDMenuSM, NULL);

	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 100;
	Offscreen.dwHeight = 24;
	pDD7->CreateSurface(&Offscreen, &pDDSMNewGame, NULL);

	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 123;
	Offscreen.dwHeight = 21;
	pDD7->CreateSurface(&Offscreen, &pDDSMCustom, NULL);

	Offscreen.dwWidth = 387;
	Offscreen.dwHeight = 234;
	pDD7->CreateSurface(&Offscreen, &pDDOptions, NULL);

	Offscreen.dwWidth = 85;
	Offscreen.dwHeight = 33;
	pDD7->CreateSurface(&Offscreen, &pDDOptionsOKP, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOptionsOKHover, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDCustomOKP, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDCustomOKHover, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDErrorOKP, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDErrorOKHover, NULL);

	Offscreen.dwWidth = 15;
	Offscreen.dwHeight = 17;
	pDD7->CreateSurface(&Offscreen, &pDDOptionsBtn1, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOptionsBtn2, NULL);
	
	Offscreen.dwWidth = 387;
	Offscreen.dwHeight = 140;
	pDD7->CreateSurface(&Offscreen, &pDDCustom, NULL);

	Offscreen.dwWidth = 193;
	Offscreen.dwHeight = 107;
	pDD7->CreateSurface(&Offscreen, &pDDError, NULL);
	
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
	pDD7->CreateSurface(&Offscreen, &pDDCCenterMask, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDSprite185x160, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDCCenterBuild, NULL);
	pDDCCenterBuild->SetColorKey(DDCKEY_SRCBLT, &key);

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
	Offscreen.dwWidth = 800;
	Offscreen.dwHeight = 600;
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
	pDD7->CreateSurface(&Offscreen, &pDDCCenterButton, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDCCenterButtonPressed,NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOilPlantButton, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOilPlantButtonPressed,NULL);
	pDD7->CreateSurface(&Offscreen, &pDDAirportButton, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDAirportButtonPressed,NULL);
	pDD7->CreateSurface(&Offscreen, &pDDPyramidButton, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDPyramidButtonPressed,NULL);


	Offscreen.dwWidth = 295;
	Offscreen.dwHeight = 212;
	pDD7->CreateSurface(&Offscreen, &pDDSpriteOilPlant, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOilPlantSelected, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOilPlantMask, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOilBuild, NULL);
	pDDOilBuild->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.dwWidth = 180;
	Offscreen.dwHeight = 160;
	pDD7->CreateSurface(&Offscreen, &pDDPyramid, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDPyramidSelected, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDPyramidMask, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDPyrBuild, NULL);
	pDDPyrBuild->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.dwWidth = 290;
	Offscreen.dwHeight = 265;
	pDD7->CreateSurface(&Offscreen, &pDDAirport, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDAirportSelected, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDAirportMask, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDAirBuild, NULL);
	pDDAirBuild->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.dwWidth = 80;
	Offscreen.dwHeight = 80;
	pDD7->CreateSurface(&Offscreen, &pDDGreenFrame, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDRedFrame, NULL);
	pDDGreenFrame->SetColorKey(DDCKEY_SRCBLT, &key);
	pDDRedFrame->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.dwWidth = 40;
	Offscreen.dwHeight = 40;
	pDD7->CreateSurface(&Offscreen, &pDDMissile, NULL);
	pDDMissile->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.dwWidth = 282;
	Offscreen.dwHeight = 361;
	pDD7->CreateSurface(&Offscreen, &pDDPauseMenu, NULL);

	Offscreen.dwWidth = 165;
	Offscreen.dwHeight = 45;
	pDD7->CreateSurface(&Offscreen, &pDDPauseOptions, NULL);
	
	Offscreen.dwWidth = 260;
	Offscreen.dwHeight = 70;
	pDD7->CreateSurface(&Offscreen, &pDDPauseEMenu, NULL);
	
	Offscreen.dwWidth = 195;
	Offscreen.dwHeight = 40;
	pDD7->CreateSurface(&Offscreen, &pDDPauseEGame, NULL);
	
	Offscreen.dwWidth = 185;
	Offscreen.dwHeight = 45;
	pDD7->CreateSurface(&Offscreen, &pDDPauseCont, NULL);
	
	Offscreen.dwWidth = 387;
	Offscreen.dwHeight = 234;
	pDD7->CreateSurface(&Offscreen, &pDDOptions, NULL);

	Offscreen.dwWidth = 85;
	Offscreen.dwHeight = 33;
	pDD7->CreateSurface(&Offscreen, &pDDOptionsOKP, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOptionsOKHover, NULL);
	
	Offscreen.dwWidth = 15;
	Offscreen.dwHeight = 17;
	pDD7->CreateSurface(&Offscreen, &pDDOptionsBtn1, NULL);
	pDD7->CreateSurface(&Offscreen, &pDDOptionsBtn2, NULL);

	return TRUE;
}

void DirectDrawUnInit(int iType)
{
	if (iType==UNINIT_GAME)
	{
		RELEASE(pDDOptionsBtn2);
		RELEASE(pDDOptionsBtn1);
		RELEASE(pDDOptionsOKHover);
		RELEASE(pDDOptionsOKP);
		RELEASE(pDDOptions);
		RELEASE(pDDPauseCont);
		RELEASE(pDDPauseEGame);
		RELEASE(pDDPauseEMenu);
		RELEASE(pDDPauseOptions);
		RELEASE(pDDPauseMenu);
		RELEASE(pDDMissile);
		RELEASE(pDDRedFrame);
		RELEASE(pDDGreenFrame);
		RELEASE(pDDAirBuild);
		RELEASE(pDDAirportMask);
		RELEASE(pDDAirportSelected);
		RELEASE(pDDAirport);
		RELEASE(pDDPyrBuild);
		RELEASE(pDDPyramidMask);
		RELEASE(pDDPyramidSelected);
		RELEASE(pDDPyramid);
		RELEASE(pDDOilBuild);
		RELEASE(pDDOilPlantMask);
		RELEASE(pDDOilPlantSelected);
		RELEASE(pDDSpriteOilPlant);
		RELEASE(pDDPyramidButtonPressed);
		RELEASE(pDDPyramidButton);
		RELEASE(pDDAirportButtonPressed);
		RELEASE(pDDAirportButton);
		RELEASE(pDDOilPlantButtonPressed);
		RELEASE(pDDOilPlantButton);
		RELEASE(pDDCCenterButtonPressed);
		RELEASE(pDDCCenterButton);
		RELEASE(pDDF15ButtonPressed);
		RELEASE(pDDF15Button);
		RELEASE(pDDPlaneButtonPressed);
		RELEASE(pDDPlaneButton);
		RELEASE(pDDMenuButtonOver);
		RELEASE(pDDMenuButtonPressed);
		RELEASE(pDDSprite170x160);
		RELEASE(pDDPanel);
		RELEASE(pDDCursor);
		RELEASE(pDDOffscreen);
		RELEASE(pDDUnitSelection);
		RELEASE(pDDCCenterBuild);
		RELEASE(pDDSprite185x160);
		RELEASE(pDDCCenterMask);
		RELEASE(pDDCCenterSelected);
		for (int i=65; i>=0; i--)
				RELEASE(pDDSprite120x90[i]);
		for (i=MAX_TILES-1; i>=0; i--)
			RELEASE(pDDTile[i]);
	}

	if (iType==UNINIT_MENU)
	{
		RELEASE(pDDError);
		RELEASE(pDDCustom);
		RELEASE(pDDOptionsBtn2);
		RELEASE(pDDOptionsBtn1);
		RELEASE(pDDErrorOKHover);
		RELEASE(pDDErrorOKP);
		RELEASE(pDDCustomOKHover);
		RELEASE(pDDCustomOKP);
		RELEASE(pDDOptionsOKHover);
		RELEASE(pDDOptionsOKP);
		RELEASE(pDDOptions);
		RELEASE(pDDSMCustom);
		RELEASE(pDDSMNewGame);
		RELEASE(pDDMenuSM);
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

void Loading()
{
	HDC hdc;
	HRESULT hr;
	RECT rSrcRect;

	pDDOffscreen->GetDC(&hdc);
	Load.Draw(hdc);
	pDDOffscreen->ReleaseDC(hdc);
	SetRect(&rSrcRect,0,0,640,480);
	hr = pDDBackBuffer->Blt(NULL, pDDOffscreen, &rSrcRect, DDBLT_WAIT, NULL);
	hr = pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	if (hr!=DD_OK) PostQuitMessage(0);
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

void PlayMusic(LPSTR szFile)
{
	WCHAR wFile[MAX_PATH];
	
	MultiByteToWideChar( CP_ACP, 0, szFile, -1, wFile, MAX_PATH);
	RELEASE(pimexMusic);
	RELEASE(pimcMusic);
	RELEASE(pigbMusic);
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pigbMusic);
	pigbMusic->QueryInterface(IID_IMediaControl, (void **)&pimcMusic);
	pigbMusic->QueryInterface(IID_IMediaEventEx, (void **)&pimexMusic);
	pigbMusic->RenderFile(wFile, NULL);
	pimcMusic->Run();
}

void DarkenScreen()
{
	DDSURFACEDESC2 sDesc;
	sDesc.dwSize=sizeof(sDesc);
	
	if (pDDOffscreen->Lock(NULL,&sDesc,DDLOCK_WAIT,NULL)==DD_OK)
	{
		PBYTE mem=(PBYTE) sDesc.lpSurface;
		for (int relx=0; relx<iResX;relx++)
			for (int rely=0; rely<iResY; rely++)
			{
				int poz=rely*sDesc.lPitch+relx*sDesc.ddpfPixelFormat.dwRGBBitCount/8;
				USHORT *m=(USHORT *) &mem[poz];
				*m= ((*m)>>1)&0x7BEF;
			}

		pDDOffscreen->Unlock(NULL);
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

	Options.Load("data\\Menu\\options.bmp");
	pDDOptions->GetDC(&hdc);
	Options.Draw(hdc);
	pDDOptions->ReleaseDC(hdc);

	bmpOK.Load("data\\Menu\\okp.bmp");
	pDDOptionsOKP->GetDC(&hdc);
	bmpOK.Draw(hdc);
	pDDOptionsOKP->ReleaseDC(hdc);

	bmpOKHover.Load("data\\Menu\\okhover.bmp");
	pDDOptionsOKHover->GetDC(&hdc);
	bmpOKHover.Draw(hdc);
	pDDOptionsOKHover->ReleaseDC(hdc);

	bmpBtn1.Load("data\\Menu\\btn1.bmp");
	pDDOptionsBtn1->GetDC(&hdc);
	bmpBtn1.Draw(hdc);
	pDDOptionsBtn1->ReleaseDC(hdc);

	bmpBtn2.Load("data\\Menu\\btn2.bmp");
	pDDOptionsBtn2->GetDC(&hdc);
	bmpBtn2.Draw(hdc);
	pDDOptionsBtn2->ReleaseDC(hdc);

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

	CBmp bmp;
	bmp.Load("data\\Menu\\startgame.bmp");
	pDDMenuBeginTXT->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDMenuBeginTXT->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\startgamesel.bmp");
	pDDMenuBeginTXTSel->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDMenuBeginTXTSel->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\optionsel.bmp");
	pDDMenuOptTXTSel->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDMenuOptTXTSel->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\exitsel.bmp");
	pDDMenuExitTXTSel->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDMenuExitTXTSel->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\startmenu.bmp");
	pDDMenuSM->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDMenuSM->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\newgame.bmp");
	pDDSMNewGame->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDSMNewGame->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\custom.bmp");
	pDDSMCustom->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDSMCustom->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\cdialog.bmp");
	pDDCustom->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDCustom->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\cokp.bmp");
	pDDCustomOKP->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDCustomOKP->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\cokhover.bmp");
	pDDCustomOKHover->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDCustomOKHover->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\error.bmp");
	pDDError->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDError->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\eokp.bmp");
	pDDErrorOKP->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDErrorOKP->ReleaseDC(hdc);

	bmp.Load("data\\Menu\\eokhover.bmp");
	pDDErrorOKHover->GetDC(&hdc);
	bmp.Draw(hdc);
	pDDErrorOKHover->ReleaseDC(hdc);
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
	
	Loading();
	LoadMenuFiles();
	
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pigbMusic);
	pigbMusic->QueryInterface(IID_IMediaControl, (void **)&pimcMusic);
	pigbMusic->QueryInterface(IID_IMediaEventEx, (void **)&pimexMusic);
	/*Music=1;
	PlayMusic("data\\music\\cello.mp3");*/

	while (UpdateGame(hwnd));

	RELEASE(pimexMusic);
	RELEASE(pimcMusic);
	RELEASE(pigbMusic);

	DirectInputUnInit();
	if ((State==GAME_ACTIVE)||(State==GAME_PAUSED))
		DirectDrawUnInit(UNINIT_GAME);
	else if (State==MAIN_MENU)
		DirectDrawUnInit(UNINIT_MENU);
	return FALSE;
}

BOOL UpdateMainMenu()
{
	RECT DestRect, rBeginGame, rOptions, rExit;
	int Selected = 0;
	static int c = 0;
	DIMOUSESTATE dims;
	HDC hdc;
	BOOL MEnter = FALSE;
	
	pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
	pDIKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
	
	CurX+=dims.lX * MouseSensitivity;
	CurY+=dims.lY * MouseSensitivity;

	POINT pCursor = {CurX, CurY};

	pDDOffscreen->GetDC(&hdc);
	MenuBack.Draw(hdc);
	SetTextAlign(hdc, TA_BOTTOM | TA_RIGHT);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255,255,255));
	TextOut(hdc, 640, 480, VERSION, strlen(VERSION));
	pDDOffscreen->ReleaseDC(hdc);
	pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);

	SetRect(&rBeginGame, 40, 170, 40 + 230, 170 + 240);
	SetRect(&rOptions, 380, 210, 380+165, 210 + 200);
	SetRect(&rExit, 280, 440, 280 + 80, 440 + 30);

	if (CurX<0) CurX = 0;
	if (CurY<0) CurY = 0;
	if (CurX>640) CurX = 640;
	if (CurY>480) CurY = 480;

	if (PtInRect(&rBeginGame, pCursor)) Selected = 1;
	if (PtInRect(&rOptions, pCursor)) Selected = 2;
	if (PtInRect(&rExit, pCursor)) Selected = 3;
	if (Selected == 1)
	{
		c++;
		if (c % 2 == 0) BGActual = BGActual->next;
	}
	if (Selected == 2) OptActual = OptActual->next;

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

	if (dims.rgbButtons[0] & 0x80) MEnter = TRUE;
	if ((Selected == 1) && MEnter) 
	{
		State = NEW_GAME;
		pDDOffscreen->Blt(NULL, pDDBackBuffer, NULL, DDBLT_WAIT, NULL);
		DarkenScreen();
	}
	if ((Selected == 2) && MEnter)
	{
		State=MENU_OPTIONS;
		pDDOffscreen->Blt(NULL, pDDBackBuffer, NULL, DDBLT_WAIT, NULL);
		DarkenScreen();
	}
	if (KEYDOWN(buffer, DIK_ESCAPE)||((Selected == 3) && MEnter)) return FALSE;

	SetRect(&DestRect, CurX, CurY, CurX + 32, CurY + 32);
	pDDBackBuffer->Blt(&DestRect, pDDCursor, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

	pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	
	return TRUE;
}

BOOL UpdateStartMenu(HWND hwnd)
{
	HDC hdc;
	DIMOUSESTATE dims;
	RECT DestRect;
	static int EscapeOld = 0;
	int Selected = 0;
	RECT rCreate, rJoin;
	BOOL MEnter = FALSE;
	
	pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
	pDIKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
	
	CurX+=dims.lX * MouseSensitivity;
	CurY+=dims.lY * MouseSensitivity;

	POINT pCursor = {CurX, CurY};
	
	if (CurX<0) CurX = 0;
	if (CurY<0) CurY = 0;
	if (CurX>640) CurX = 640;
	if (CurY>480) CurY = 480;

	SetRect(&rCreate, 270, 245, 270 + 100, 245 + 24);
	SetRect(&rJoin, 259, 283, 259 + 123, 283 + 21);
	if (PtInRect(&rCreate, pCursor)) Selected = 1;
	if (PtInRect(&rJoin, pCursor)) Selected = 2;

	pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);
	SetRect(&DestRect, 259, 223, 259 + 123, 223 + 101);
	pDDBackBuffer->Blt(&DestRect, pDDMenuSM, NULL, DDBLT_WAIT, NULL);

	if (!KEYDOWN(buffer, DIK_ESCAPE)&&EscapeOld) State=MAIN_MENU;
	EscapeOld=KEYDOWN(buffer, DIK_ESCAPE);

	if (Selected==1)
		pDDBackBuffer->Blt(&rCreate, pDDSMNewGame, NULL, DDBLT_WAIT, NULL);
	if (Selected==2)
		pDDBackBuffer->Blt(&rJoin, pDDSMCustom, NULL, DDBLT_WAIT, NULL);
	SetRect(&DestRect, CurX, CurY, CurX + 32, CurY + 32);
	pDDBackBuffer->Blt(&DestRect, pDDCursor, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

	pDDPrimary->Flip(NULL, DDFLIP_WAIT);

	if (dims.rgbButtons[0] & 0x80) MEnter = TRUE;
	if (MEnter)
		switch (Selected)
		{
		case 1:
			State=GAME_ACTIVE;
			DirectDrawUnInit(UNINIT_MENU);
			DirectDrawInit(800,600,hwnd);
			CreateGameOffscreenSurfaces();
			Loading();
			Engine.Load("data\\Maps\\map.lwm");
			pDDCursor->GetDC(&hdc);
			Cursor.Draw(hdc);
			pDDCursor->ReleaseDC(hdc);
			break;
		case 2:
			State=NEW_CUSTOM_GAME;
			EnterDialog=TRUE;
			iFileNameLen=0;
			break;
		}
	return TRUE;
}

BOOL UpdateOptions(int lx, int ty)
{
	RECT DestRect, rOK, rLow, rNormal, rHigh, rCello, rRock, rOff;
	RECT rBtnLow, rBtnNormal, rBtnHigh, rBtnCello, rBtnRock, rBtnOff;
	DIMOUSESTATE dims;
	static BOOL bLMB=FALSE, bLMBOld=FALSE, bOKP=FALSE, bOKPOld=FALSE;
	static BOOL bLow=FALSE, bLowOld=FALSE, bNormal=FALSE, bNormalOld=FALSE, bHigh=FALSE, bHighOld=FALSE;
	static BOOL bCello=FALSE, bCelloOld=FALSE, bRock=FALSE, bRockOld=FALSE, bOff=FALSE, bOffOld=FALSE;

	pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
	
	CurX+=dims.lX * MouseSensitivity;
	CurY+=dims.lY * MouseSensitivity;
	if (dims.rgbButtons[0] & 0x80) bLMB=TRUE;
	else bLMB=FALSE;

	if (!bLMB) bOKP=bLow=bHigh=bNormal=bCello=bRock=bOff=FALSE;

	POINT pCursor = {CurX, CurY};
	if (CurX<0) CurX = 0;
	if (CurY<0) CurY = 0;
	if (CurX>640) CurX = 640;
	if (CurY>480) CurY = 480;

	SetRect(&DestRect, lx, ty, lx+387, ty+234);
	SetRect(&rOK, lx+159,ty+186,lx+159+85,ty+186+33);
	SetRect(&rLow,lx+40,ty+82,lx+40+42,ty+82+14);
	SetRect(&rNormal,lx+151,ty+82,lx+151+60,ty+82+14);
	SetRect(&rHigh,lx+280,ty+82,lx+280+41,ty+82+17);
	SetRect(&rCello,lx+40,ty+148,lx+40+47,ty+148+14);
	SetRect(&rRock,lx+151,ty+148,lx+151+44,ty+148+14);
	SetRect(&rOff,lx+280,ty+148,lx+280+35,ty+148+14);
	SetRect(&rBtnLow,lx+39, ty+82, lx+39+15, ty+82+17);
	SetRect(&rBtnNormal,lx+150, ty+82, lx+150+15, ty+82+17);
	SetRect(&rBtnHigh,lx+278, ty+82, lx+278+15, ty+82+17);
	SetRect(&rBtnCello,lx+39, ty+148, lx+39+15, ty+148+17);
	SetRect(&rBtnRock,lx+150, ty+148, lx+150+15, ty+148+17);
	SetRect(&rBtnOff,lx+278, ty+148, lx+278+15, ty+148+17);

	pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);
	pDDBackBuffer->Blt(&DestRect, pDDOptions, NULL, DDBLT_WAIT, NULL);

	if (PtInRect(&rOK, pCursor))
		if (bLMB)
		{
			if (!bLMBOld) bOKP=TRUE;
			if (bOKP) pDDBackBuffer->Blt(&rOK, pDDOptionsOKP, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rOK, pDDOptionsOKHover, NULL, DDBLT_WAIT, NULL);
	if (PtInRect(&rOK,pCursor)&&bOKPOld&&(!bLMB)) 
	{
		if (State==MENU_OPTIONS) State=MAIN_MENU;
		else if (State=GAME_OPTIONS) State=GAME_PAUSED;
	}

	if (PtInRect(&rLow, pCursor)) if (bLMB) if (!bLMBOld) bLow=TRUE;
	if (PtInRect(&rNormal, pCursor)) if (bLMB) if (!bLMBOld) bNormal=TRUE;
	if (PtInRect(&rHigh, pCursor)) if (bLMB) if (!bLMBOld) bHigh=TRUE;
	if (PtInRect(&rCello, pCursor)) if (bLMB) if (!bLMBOld) bCello=TRUE;
	if (PtInRect(&rRock, pCursor)) if (bLMB) if (!bLMBOld) bRock=TRUE;
	if (PtInRect(&rOff, pCursor)) if (bLMB) if (!bLMBOld) bOff=TRUE;

	if (PtInRect(&rLow,pCursor)&&bLowOld&&(!bLMB)) MouseSensitivity=1;
	if (PtInRect(&rNormal,pCursor)&&bNormalOld&&(!bLMB)) MouseSensitivity=2;
	if (PtInRect(&rHigh,pCursor)&&bHighOld&&(!bLMB)) MouseSensitivity=3;
	if (PtInRect(&rCello,pCursor)&&bCelloOld&&(!bLMB)) 
	{
		if (Music!=3) pimcMusic->Stop();
		Music=1;
		PlayMusic("data\\music\\cello.mp3");
	}
	if (PtInRect(&rRock,pCursor)&&bRockOld&&(!bLMB))
	{
		if (Music!=3) pimcMusic->Stop();
		Music=2;
		PlayMusic("data\\music\\rock.mp3");
	}
	if (PtInRect(&rOff,pCursor)&&bOffOld&&(!bLMB)) 
	{
		if (Music!=3) pimcMusic->Stop();
		Music=3;
	}

	switch (MouseSensitivity)
	{
	case 1:
		pDDBackBuffer->Blt(&rBtnLow, pDDOptionsBtn1, NULL, DDBLT_WAIT, NULL);
		break;
	case 2:
		pDDBackBuffer->Blt(&rBtnNormal, pDDOptionsBtn1, NULL, DDBLT_WAIT, NULL);
		break;
	case 3:
		pDDBackBuffer->Blt(&rBtnHigh, pDDOptionsBtn1, NULL, DDBLT_WAIT, NULL);
		break;
	}

	switch (Music)
	{
	case 1:
		pDDBackBuffer->Blt(&rBtnCello, pDDOptionsBtn2, NULL, DDBLT_WAIT, NULL);
		break;
	case 2:
		pDDBackBuffer->Blt(&rBtnRock, pDDOptionsBtn2, NULL, DDBLT_WAIT, NULL);
		break;
	case 3:
		pDDBackBuffer->Blt(&rBtnOff, pDDOptionsBtn2, NULL, DDBLT_WAIT, NULL);
		break;
	}

	SetRect(&DestRect, CurX, CurY, CurX + 32, CurY + 32);
	pDDBackBuffer->Blt(&DestRect, pDDCursor, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	bLMBOld=bLMB;
	bOKPOld=bOKP;
	bLowOld=bLow;
	bNormalOld=bNormal;
	bHighOld=bHigh;
	bCelloOld=bCello;
	bRockOld=bRock;
	bOffOld=bOff;
	return TRUE;
}

BOOL UpdateStartCustom(HWND hwnd)
{
	HDC hdc;
	FILE *in;
	RECT DestRect, rOK;
	DIMOUSESTATE dims;
	static BOOL bLMB=FALSE, bLMBOld=FALSE, bOKP=FALSE, bOKPOld=FALSE, back, prev_back;
	static char map_name[35], key, prev_key;

	if (EnterDialog) 
	{
		prev_key=NULL;
		prev_back=NULL;
	}
	key=NULL;
	back=NULL;

	pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
	pDIKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
	
	CurX+=dims.lX * MouseSensitivity;
	CurY+=dims.lY * MouseSensitivity;
	if (dims.rgbButtons[0] & 0x80) bLMB=TRUE;
	else bLMB=FALSE;

	if (!bLMB) bOKP=FALSE;

	POINT pCursor = {CurX, CurY};

	if (CurX<0) CurX = 0;
	if (CurY<0) CurY = 0;
	if (CurX>640) CurX = 640;
	if (CurY>480) CurY = 480;

	SetRect(&DestRect, 126, 170, 126+387, 170+140);
	SetRect(&rOK, 126+155,170+101,126+155+85,170+101+33);

	pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);
	pDDBackBuffer->Blt(&DestRect, pDDCustom, NULL, DDBLT_WAIT, NULL);

	if (KEYDOWN(buffer, DIK_A)) key='a';
	if (KEYDOWN(buffer, DIK_S)) key='s';
	if (KEYDOWN(buffer, DIK_D)) key='d';
	if (KEYDOWN(buffer, DIK_F)) key='f';
	if (KEYDOWN(buffer, DIK_G)) key='g';
	if (KEYDOWN(buffer, DIK_H)) key='h';
	if (KEYDOWN(buffer, DIK_J)) key='j';
	if (KEYDOWN(buffer, DIK_K)) key='k';
	if (KEYDOWN(buffer, DIK_L)) key='l';
	if (KEYDOWN(buffer, DIK_Z)) key='z';
	if (KEYDOWN(buffer, DIK_X)) key='x';
	if (KEYDOWN(buffer, DIK_C)) key='c';
	if (KEYDOWN(buffer, DIK_V)) key='v';
	if (KEYDOWN(buffer, DIK_B)) key='b';
	if (KEYDOWN(buffer, DIK_N)) key='n';
	if (KEYDOWN(buffer, DIK_M)) key='m';
	if (KEYDOWN(buffer, DIK_Q)) key='q';
	if (KEYDOWN(buffer, DIK_W)) key='w';
	if (KEYDOWN(buffer, DIK_E)) key='e';
	if (KEYDOWN(buffer, DIK_R)) key='r';
	if (KEYDOWN(buffer, DIK_T)) key='t';
	if (KEYDOWN(buffer, DIK_Y)) key='y';
	if (KEYDOWN(buffer, DIK_U)) key='u';
	if (KEYDOWN(buffer, DIK_I)) key='i';
	if (KEYDOWN(buffer, DIK_O)) key='o';
	if (KEYDOWN(buffer, DIK_P)) key='p';
	if (KEYDOWN(buffer, DIK_1)) key='1';
	if (KEYDOWN(buffer, DIK_2)) key='2';
	if (KEYDOWN(buffer, DIK_3)) key='3';
	if (KEYDOWN(buffer, DIK_4)) key='4';
	if (KEYDOWN(buffer, DIK_5)) key='5';
	if (KEYDOWN(buffer, DIK_6)) key='6';
	if (KEYDOWN(buffer, DIK_7)) key='7';
	if (KEYDOWN(buffer, DIK_8)) key='8';
	if (KEYDOWN(buffer, DIK_9)) key='9';
	if (KEYDOWN(buffer, DIK_0)) key='0';

	if (key) if (key!=prev_key&&iFileNameLen<=30) map_name[iFileNameLen++]=key;
	map_name[iFileNameLen]=NULL;
	if (KEYDOWN(buffer, DIK_BACK))
	{
		back=TRUE;
		if (back!=prev_back)
		{
			iFileNameLen--;
			if (iFileNameLen<0) iFileNameLen=0;
			map_name[iFileNameLen]=NULL;
		}
	}
	pDDBackBuffer->GetDC(&hdc);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255,255,255));
	TextOut(hdc, 230, 232, map_name, strlen(map_name));
	pDDBackBuffer->ReleaseDC(hdc);

	if (PtInRect(&rOK, pCursor))
		if (bLMB)
		{
			if (!bLMBOld&&!EnterDialog) bOKP=TRUE;
			if (bOKP) pDDBackBuffer->Blt(&rOK, pDDCustomOKP, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rOK, pDDCustomOKHover, NULL, DDBLT_WAIT, NULL);
	if (PtInRect(&rOK,pCursor)&&bOKPOld&&(!bLMB)) 
	{
		char filename[50]="data\\maps\\";
		for (unsigned int i=0; i<strlen(map_name); i++) filename[i+10]=map_name[i];
		i=10+strlen(map_name);
		filename[i]='.'; filename[i+1]='l';filename[i+2]='w'; filename[i+3]='m';
		filename[i+4]=NULL;
		in = fopen(filename, "rb");
		if (in)
		{
			State=GAME_ACTIVE;
			DirectDrawUnInit(UNINIT_MENU);
			DirectDrawInit(800,600,hwnd);
			CreateGameOffscreenSurfaces();
			Loading();
			Engine.Load(filename);
			pDDCursor->GetDC(&hdc);
			Cursor.Draw(hdc);
			pDDCursor->ReleaseDC(hdc);
			fclose(in);
		}
		else State=NEW_GAME_ERROR;
	}

	SetRect(&DestRect, CurX, CurY, CurX + 32, CurY + 32);
	pDDBackBuffer->Blt(&DestRect, pDDCursor, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	bLMBOld=bLMB;
	bOKPOld=bOKP;
	EnterDialog=FALSE;
	prev_key=key;
	prev_back=back;
	return TRUE;
}

BOOL UpdateNewGameError()
{
	RECT DestRect, rOK;
	DIMOUSESTATE dims;
	static BOOL bLMB=FALSE, bLMBOld=FALSE, bOKP=FALSE, bOKPOld=FALSE;
	
	pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
	
	CurX+=dims.lX * MouseSensitivity;
	CurY+=dims.lY * MouseSensitivity;
	if (dims.rgbButtons[0] & 0x80) bLMB=TRUE;
	else bLMB=FALSE;

	if (!bLMB) bOKP=FALSE;

	POINT pCursor = {CurX, CurY};
	if (CurX<0) CurX = 0;
	if (CurY<0) CurY = 0;
	if (CurX>640) CurX = 640;
	if (CurY>480) CurY = 480;

	SetRect(&DestRect, 223, 186, 223+193, 186+107);
	SetRect(&rOK, 223+55,186+69,223+55+85,186+69+33);
	
	pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);
	pDDBackBuffer->Blt(&DestRect, pDDError, NULL, DDBLT_WAIT, NULL);

	if (PtInRect(&rOK, pCursor))
		if (bLMB)
		{
			if (!bLMBOld) bOKP=TRUE;
			if (bOKP) pDDBackBuffer->Blt(&rOK, pDDErrorOKP, NULL, DDBLT_WAIT, NULL);
		}
		else pDDBackBuffer->Blt(&rOK, pDDErrorOKHover, NULL, DDBLT_WAIT, NULL);
	if (PtInRect(&rOK,pCursor)&&bOKPOld&&(!bLMB)) State=NEW_GAME;

	SetRect(&DestRect, CurX, CurY, CurX + 32, CurY + 32);
	pDDBackBuffer->Blt(&DestRect, pDDCursor, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	bLMBOld=bLMB;
	bOKPOld=bOKP;
	return TRUE;
}

int UpdateGame(HWND hwnd)
{
	if (Music!=3)
		if (SUCCEEDED(pimexMusic->GetEvent(&evCode, &evParam1, &evParam2, 0)))
		{
			pimexMusic->FreeEventParams(evCode, evParam1, evParam2);
			if ((EC_COMPLETE == evCode) || (EC_USERABORT == evCode))
			{
				pimcMusic->Stop();
				if (Music==1)
				{
					Music=2;
					PlayMusic("data\\music\\rock.mp3");
				}
				else if (Music==2)
				{
					Music=1;
					PlayMusic("data\\music\\cello.mp3");
				}
			}
		}

	switch (State)
	{
		case GAME_ACTIVE:
			if (!Engine.Update()) return 0;
			break;
		case GAME_PAUSED:
			if (!Engine.UpdatePauseMenu(hwnd)) return 0;
			break;
		case MAIN_MENU:
			if (!UpdateMainMenu()) return 0;
			break;
		case NEW_GAME:
			if (!UpdateStartMenu(hwnd)) return 0;
			break;
		case NEW_CUSTOM_GAME:
			if (!UpdateStartCustom(hwnd)) return 0;
			break;
		case MENU_OPTIONS:
			if (!UpdateOptions(126,123)) return 0;
			break;
		case GAME_OPTIONS:
			if (!UpdateOptions(206,183)) return 0;
			break;
		case NEW_GAME_ERROR:
			if (!UpdateNewGameError()) return 0;
			break;
	}
	return 1;
}