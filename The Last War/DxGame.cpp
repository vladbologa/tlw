/*----------------------------------------------
  DxGame.cpp - "The Last War" main source file
		(c) 1999-2001 Vlad Bologa
------------------------------------------------*/

#include "stdafx.h"
#include "Bmp.h"

#include "map.h"
#include "unit.h"

#include <stdio.h>
#include <mmsystem.h>
#include <streams.h>
#include <string.h>

#include <ddraw.h>
#include <dinput.h>

#define  VERSION "v0.2.4b"

#define KEYDOWN(name,key) (name[key] & 0x80)
#define WM_GRAPHNOTIFY  WM_USER+13
#define HELPER_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define PLAYING TRUE
#define STOPPED FALSE

#define LBT_BLACK 0
#define LBT_TRANSPARENT 1
#define LM_STATIC 0
#define LM_DINAMIC 1

#define MAX_TILES 10

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
char     buffer[256];
int NewGameCounter = 0;

int MouseSensitiveness = 200;

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
LPDIRECTDRAWSURFACE7 pDDOffscreen2 = NULL;
LPDIRECTDRAWSURFACE7 pDDPanel = NULL;
LPDIRECTDRAWSURFACE7 pDDTile[MAX_TILES];
LPDIRECTDRAWSURFACE7 pDDMenuBegin, pDDMenuOpt;
LPDIRECTDRAWSURFACE7 pDDSprite120x90[33];

//Function prototypes
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
BOOL BeginGame(HWND);
BOOL DirectDrawInit(HWND);
BOOL DirectInputInit(HWND);
BOOL TileInScreen(RECT);
void DirectDrawUnInit();
void DirectInputUnInit();
void CALLBACK Actualizare(HWND, UINT, UINT, DWORD);
void UpdateGame();
void UpdateMainMenu();
void LoadMenuFiles();
void PlayFile(LPSTR, HWND);
void Loading(int, int);
void InsertImage();

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
	int CurentX, CurentY;
	int mouse_x, mouse_y, fmouse_x, fmouse_y;
	BOOL LeftButtonPressed, RightButtonPressed;
	BOOL IsSelecting, WaitSelection;
	CMap Map;
	CBmp dBar, TerrainType[256];
	BOOL fLMBPressed, oldLMBPressed;
	int UnitCount;

	CUnit *plane, *first, *temp;
public:
	void GetMouseCoords(int &x, int &y);
	void ShowMouse();
	void CorrectCoords();
	BOOL Update(int Reserved = 0);
	BOOL UpdateTerrain(int x = 0, int y = 0);
	BOOL Load(int Level, int Reserved = 0);
	void LoadTerrainTiles(int tileset = 0);
	GameEngine();
	~GameEngine();
};

BOOL GameEngine::Update(int Reserved)
{
	int m_x, m_y;
	HDC hdc;
	static HPEN hPen=CreatePen(PS_SOLID, 1, RGB(20,200,40));
	static int add=0;

	GetMouseCoords(m_x, m_y);
	pDIKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);

//***********Scrolling***********
	if (!IsSelecting)
	{
		if (m_x == 640)
			CurentX+=10;
		if (m_x == 0) CurentX-=10;
		if (m_y == 480) 
			CurentY+=10;
		if (m_y == 0) CurentY-=10;
	}
	CorrectCoords();
	UpdateTerrain(CurentX, CurentY);
//********End of Scrolling********

	if (IsSelecting&&(!LeftButtonPressed)) 
	{
		plane=first;
		while (plane)
		{
			POINT pt;
			LONG tmp;
			RECT rc, r_plane, temp;

			pt.x=plane->GetX()-CurentX;
			pt.y=plane->GetY()-CurentY;
			rc.top=fmouse_y;
			rc.left=fmouse_x;
			rc.bottom=mouse_y;
			rc.right=mouse_x;

			r_plane.top=pt.y;
			r_plane.left=pt.x;
			r_plane.bottom=pt.y+90;
			r_plane.right=pt.x+120;

			if (rc.top>rc.bottom){ tmp=rc.top; rc.top=rc.bottom; rc.bottom=tmp;}
			if (rc.left>rc.right){ tmp=rc.left; rc.left=rc.right; rc.right=tmp;}

			if (IntersectRect(&temp, &rc, &r_plane)) plane->Select(TRUE);
			else plane->Select(FALSE);
			plane=plane->next;
		}
		IsSelecting=FALSE;
		WaitSelection=FALSE;
	}
	if (WaitSelection&&!LeftButtonPressed)
	{
		BOOL sel=FALSE;

		plane=first;
		while (plane->next) plane=plane->next;
		while (plane)
		{
			POINT pt;
			RECT r_plane;

			pt.x=mouse_x;
			pt.y=mouse_y;

			r_plane.top=plane->GetY()-CurentY;
			r_plane.left=plane->GetX()-CurentX;
			r_plane.bottom=r_plane.top+90;
			r_plane.right=r_plane.left+120;

			if (PtInRect(&r_plane, pt))
			{ 
				if (!sel)
				{
					plane->Select(TRUE); 
					sel=TRUE; 
				}
				else plane->Select(FALSE);
			}
			else plane->Select(FALSE);
			plane=plane->prev;
		}
		WaitSelection=FALSE;
	}

	if (LeftButtonPressed&&oldLMBPressed&&(!fLMBPressed)&&!IsSelecting) WaitSelection=TRUE;
	if (WaitSelection) if (mouse_x!=fmouse_x&&mouse_y!=fmouse_y) 
	{
		IsSelecting=TRUE;
		WaitSelection=FALSE;
	}

	plane=first;
	while (plane)
	{
		if (RightButtonPressed&&plane->Selected()) plane->SetDestination(CurentX+m_x-60+rand()%10, CurentY+m_y-45+rand()%10);
		plane->Update();

		RECT DestRect;

		SetRect(&DestRect, plane->GetX()-CurentX, plane->GetY()-CurentY, plane->GetX()+120-CurentX, plane->GetY()+90-CurentY);
		if (plane->Selected())
		{
			pDDBackBuffer->GetDC(&hdc);
			SelectObject(hdc, (HBRUSH) GetStockObject(NULL_BRUSH));
			SelectObject(hdc, hPen);
			Ellipse(hdc, DestRect.left+15, DestRect.top+15, DestRect.right-15, DestRect.bottom-15);
			pDDBackBuffer->ReleaseDC(hdc);
		}
		pDDBackBuffer->Blt(&DestRect, pDDSprite120x90[plane->GetCurrentFrame()], NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		plane=plane->next;
	}	

	if (IsSelecting)
	{
		pDDBackBuffer->GetDC(&hdc);
		SelectObject(hdc, hPen);
		SelectObject(hdc, (HBRUSH) GetStockObject(NULL_BRUSH));
		Rectangle(hdc,fmouse_x, fmouse_y, mouse_x, mouse_y);
		pDDBackBuffer->ReleaseDC(hdc);
	}
	
	RECT DestRect;
	SetRect(&DestRect,0, 480-125, 640,480);
	HRESULT hr;
	hr = pDDBackBuffer->Blt(&DestRect, pDDPanel, NULL, DDBLT_WAIT, NULL);
	if (hr!=DD_OK) return FALSE;
	
	hr = pDDBackBuffer->GetDC(&hdc);
	//if (FAILED(hr)) PostQuitMessage(0);
	SetTextAlign(hdc, TA_BOTTOM | TA_RIGHT);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255,255,255));
	char _itoa_t[10];
	_itoa(UnitCount, _itoa_t,10);
	if (IsSelecting) TextOut(hdc, 640, 480, "Selection", 9);
	else if (WaitSelection) TextOut(hdc, 640, 480, "Click", 5);
	else TextOut(hdc, 640, 480, _itoa_t, strlen(_itoa_t));
	hr = pDDBackBuffer->ReleaseDC(hdc);
	//if (FAILED(hr)) PostQuitMessage(0);

	ShowMouse();
	if (!IsSelecting&&!WaitSelection)
	{
		fmouse_x=mouse_x;
		fmouse_y=mouse_y;
		fLMBPressed=oldLMBPressed;
	}
	oldLMBPressed=LeftButtonPressed;


	pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	
	if (KEYDOWN(buffer, DIK_ESCAPE))
	{
		DeleteObject(hPen);
		return FALSE;
	}

	if (KEYDOWN(buffer, DIK_ADD))
	{
		if (!add)
		{
			if (!first)
			{
					first=(CUnit *) new CUnit;
					first->SetPosition(320,200);
					first->SetDestination(320,200);
					first->Select(TRUE);
					first->next=NULL;
					first->prev=NULL;
			}
			else
			{
				temp=(CUnit *) new CUnit;
				temp->SetPosition(320,200);
				temp->SetDestination(320,200);
				temp->Select(FALSE);
				plane=first;
				while (plane->next) plane=plane->next;
				plane->next=temp;
				temp->prev=plane;
				temp->next=NULL;
			}
			add=1;
			UnitCount++;
		}
	}
	else add=0;

	if (KEYDOWN(buffer, DIK_DELETE))
	{
		int change=0;
		do
		{
			plane=first;
			change=0;
			if (first->Selected()&&(!first->next))
			{
				delete plane;
				first=NULL;
				change=1;
			}
			else if (first->Selected())
			{
				plane=plane->next;
				plane->prev=NULL;
				delete first;
				first=plane;
				change=1;
			}
			else
			{
				while ((plane->next)&&(!plane->Selected())) plane=plane->next;
				if (plane->next)
				{
					temp=plane;
					plane=plane->prev;
					plane->next=temp->next;
					delete temp;
					temp=plane;
					plane=plane->next;
					plane->prev=temp;
					change=1;
				}
				else if (plane->Selected())
				{
					temp=plane->prev;
					temp->next=NULL;
					delete plane;
					plane=NULL;
					change=1;
				}
			}
			if (change) UnitCount--;
		}
		while (change);
	}
	return TRUE;
}

BOOL GameEngine::UpdateTerrain(int x, int y)
{
	x = -x;
	y = -y;
	
	for (int i = 0; i < Map.GetSizeY(); i++)
		for (int j = 0; j < Map.GetSizeX(); j++)
		{
			RECT DestRect;
			SetRect(&DestRect, x + (80 * j), y + (80 * i), x + (80 * j + 80), y + (80 * i + 80));
			if (TileInScreen(DestRect))
				pDDBackBuffer->Blt(&DestRect, pDDTile[Map.GetTerrainType(i, j)], NULL, DDBLT_WAIT, NULL);
		}
	return TRUE;
}

void GameEngine::CorrectCoords()
{
	if (CurentX < 0) CurentX = 0;
	if (CurentX > (Map.GetSizeX() * 80 - 640)) CurentX = Map.GetSizeX() * 80 - 640;
	if (CurentY < 0) CurentY = 0;
	if (CurentY > (Map.GetSizeY() * 80 - 480+125)) CurentY = Map.GetSizeY() * 80 - 480+125;
}

GameEngine::GameEngine()
{

}

GameEngine::Load(int Level, int Reserved)
{
	CurentX = CurentY = 0;
	IsSelecting=FALSE;
	WaitSelection=FALSE;
	Map.Load(Level);
	LoadTerrainTiles();	
	fLMBPressed=oldLMBPressed=FALSE;

	first=(CUnit *) new CUnit;
	first->SetPosition(320,200);
	first->SetDestination(320,100);
	first->Select(TRUE);
	first->next=NULL;
	first->prev=NULL;
	UnitCount=1;

	return TRUE;
}

void GameEngine::LoadTerrainTiles(int tileset)
{
	TerrainType[0].Load("c:\\GameArt\\Tiles\\grass.bmp");
	TerrainType[1].Load("c:\\GameArt\\Tiles\\treel1.bmp");
	TerrainType[2].Load("c:\\GameArt\\Tiles\\treel2.bmp");
	TerrainType[3].Load("c:\\GameArt\\Tiles\\treel3.bmp");
	TerrainType[4].Load("c:\\GameArt\\Tiles\\treel4.bmp");
	TerrainType[5].Load("c:\\GameArt\\Tiles\\treer1.bmp");
	TerrainType[6].Load("c:\\GameArt\\Tiles\\treer2.bmp");
	TerrainType[7].Load("c:\\GameArt\\Tiles\\treer3.bmp");
	TerrainType[8].Load("c:\\GameArt\\Tiles\\treer4.bmp");

	dBar.Load("c:\\GameArt\\dbar.bmp");

	HDC hdc;

	pDDPanel->GetDC(&hdc);
	dBar.Draw(hdc);
	pDDPanel->ReleaseDC(hdc);

	for (int i = 0; i < 9; i++)
	{
		HDC hdc;

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
	
	m_x+=dims.lX * MouseSensitiveness / 100;
	m_y+=dims.lY * MouseSensitiveness / 100;


	if (m_x < 0) m_x = 0;
	if (m_x > 640) m_x = 640;

	if (m_y < 0) m_y = 0;
	if (m_y > 480) m_y = 480;
	
	x = m_x; y = m_y;
	mouse_x = m_x; mouse_y = m_y;
	if (dims.rgbButtons[0] & 0x80) LeftButtonPressed=TRUE;
		else LeftButtonPressed=FALSE;
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

GameEngine::~GameEngine()
{
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
	 SetRect(&ScreenSize, 0, 0, 640, 480-125);

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
	 
     hwnd = CreateWindow (szAppName,
		            szAppName,
                    WS_POPUP,
                    0,
                    0,
                    GetSystemMetrics(SM_CXSCREEN),
                    GetSystemMetrics(SM_CYSCREEN),
                    NULL,
                    NULL,
                    hInstance,
		            NULL);

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
		/*if (!BeginGame(hwnd))
			PostQuitMessage(0);*/
		PlayFile("c:\\GameArt\\Mini-Intro.avi", hwnd);
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

                    HELPER_RELEASE(pivw);
                    HELPER_RELEASE(pif);
                    HELPER_RELEASE(pigb);
                    HELPER_RELEASE(pimc);
                    HELPER_RELEASE(pimex);

                    FilmState = STOPPED;
					if (!BeginGame(hwnd))
						PostQuitMessage(0);
					break;
                  }
              }
		return 0;

	case WM_DESTROY:
		DirectDrawUnInit();
		DirectInputUnInit();
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
	POINT p3 = {tile.right - 80, tile.bottom};
	POINT p4 = {tile.left + 80, tile.top};

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

BOOL DirectDrawInit(HWND hwnd)
{
	HRESULT hr = 0;
	
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
	hr = pDD7->SetDisplayMode(640, 480,16,0,0);
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
	rgndh.rcBound.left = 640;
	rgndh.rcBound.bottom = 480;

	
	SetRect(&clip, 0, 0, 640, 480);
	
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

	//Creating offscreen surfaces
	DDSURFACEDESC2 Offscreen;
	DDCOLORKEY key;
	
	ZeroMemory(&Offscreen, sizeof(DDSURFACEDESC2));
	Offscreen.dwSize=sizeof(DDSURFACEDESC2);

	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 80;
	Offscreen.dwHeight = 80;
	
	key.dwColorSpaceLowValue=0;
	key.dwColorSpaceHighValue=0;

	for (int i = 0; i < MAX_TILES; i++)
	{
		hr = pDD7->CreateSurface(&Offscreen,&pDDTile[i],NULL);
		if (hr!=DD_OK)
		{
			MessageBox(hwnd,"Error while creating offscreen surfaces. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}	

		pDDTile[i]->SetColorKey(DDCKEY_SRCBLT,&key);
	}

	
	
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 120;
	Offscreen.dwHeight = 90;
	CBmp Sprite;
	for (i = 0; i < 33; i++)
	{
		hr = pDD7->CreateSurface(&Offscreen,&pDDSprite120x90[i],NULL);
		if (hr!=DD_OK)
		{
			MessageBox(hwnd,"Error while creating offscreen surfaces. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}	
		
		pDDSprite120x90[i]->SetColorKey(DDCKEY_SRCBLT,&key);
		
		HDC hdc;
		Sprite.Load("C:\\GameArt\\Units\\Plane\\plane0.bmp");
		pDDSprite120x90[0]->GetDC(&hdc);
		if (FAILED(Sprite.Draw(hdc))) return FALSE;
		pDDSprite120x90[0]->ReleaseDC(hdc);

		for (int j = 1; j <=32; j++)
		{
			char buffer[256];
			sprintf(buffer, "C:\\GameArt\\Units\\Plane\\plane%d.bmp", i);
			Sprite.Load(buffer);

			pDDSprite120x90[i]->GetDC(&hdc);
			if (FAILED(Sprite.Draw(hdc))) return FALSE;
			pDDSprite120x90[i]->ReleaseDC(hdc);
		}	
	}

	ZeroMemory(&Offscreen, sizeof(DDSURFACEDESC2));
	Offscreen.dwSize=sizeof(DDSURFACEDESC2);

	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 640;
	Offscreen.dwHeight = 480;

	hr = pDD7->CreateSurface(&Offscreen,&pDDOffscreen,NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating offscreen surfaces. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	hr = pDD7->CreateSurface(&Offscreen,&pDDOffscreen2,NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating offscreen surfaces. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	pDDOffscreen->SetColorKey(DDCKEY_SRCBLT,&key);
	pDDOffscreen2->SetColorKey(DDCKEY_SRCBLT, &key);

	ZeroMemory(&Offscreen, sizeof(DDSURFACEDESC2));
	Offscreen.dwSize=sizeof(DDSURFACEDESC2);

	Offscreen.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 32;
	Offscreen.dwHeight = 32;

	hr = pDD7->CreateSurface(&Offscreen, &pDDCursor, NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating offscreen surfaces. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	pDDCursor->SetColorKey(DDCKEY_SRCBLT, &key);

	Offscreen.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	Offscreen.dwWidth = 220;
	Offscreen.dwHeight = 220;

	hr = pDD7->CreateSurface(&Offscreen,&pDDMenuBegin,NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating offscreen surfaces. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	Offscreen.dwWidth = 170;
	Offscreen.dwHeight = 150;

	hr = pDD7->CreateSurface(&Offscreen,&pDDMenuOpt,NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd,"Error while creating offscreen surfaces. Restart the computer and try again.","Error",MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	Offscreen.dwWidth = 640;
	Offscreen.dwHeight = 125;

	hr = pDD7->CreateSurface(&Offscreen, &pDDPanel, NULL);
	if (hr!=DD_OK)
	{
		MessageBox(hwnd, "Error while creating offscreen surfaces. Restart the computer and try again.", "Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	return TRUE;
}

void DirectDrawUnInit()
{
	pDDCursor->Release();
	for (int i = 0; i<MAX_TILES; i++)
		pDDTile[i]->Release();
	pDDOffscreen->Release();
	pDDOffscreen2->Release();
	pDDClipper->Release();
	pDDBackBuffer->Release();
	pDDPrimary->Release();
	pDD7->Release();
}

void DirectInputUnInit()
{
	pDIMouse->Unacquire();
	pDIMouse->Release();
	pDIKeyboard->Unacquire();
	pDIKeyboard->Release();
	pDI->Release();
}

void Loading(int BackgroundType, int Motion)
{
	HDC hdc;
	HRESULT hr;

	switch (Motion)
	{
	case 0:
		pDDOffscreen2->GetDC(&hdc);
		Load.Draw(hdc);
		pDDOffscreen2->ReleaseDC(hdc);

		if (BackgroundType == LBT_BLACK)
			hr = pDDBackBuffer->Blt(NULL, pDDOffscreen2, NULL, DDBLT_WAIT, NULL);
		if (BackgroundType == LBT_TRANSPARENT)
			pDDBackBuffer->Blt(NULL, pDDOffscreen2, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
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


void LoadMenuFiles()
{	
	BOOL b;

	b = MenuBack.Load("c:\\GameArt\\MainMenu.bmp");
	if (!b) PostQuitMessage(0);

	b = Cursor.Load("c:\\GameArt\\Cursor.bmp");
	
	HDC hdc;
	
	HRESULT hr = pDDCursor->GetDC(&hdc);
	if(FAILED(hr)) PostQuitMessage(0);
	b = Cursor.Draw(hdc);
	pDDCursor->ReleaseDC(hdc);

	BGActual = new CBmpList;

	BGFirst = BGActual;
	BGActual->Load("C:\\GameArt\\Menu\\BeginGame\\bg0.bmp");
	for (int i = 1; i <=25; i++)
	{
		char buffer[256];
		sprintf(buffer, "C:\\GameArt\\Menu\\BeginGame\\bg%d.bmp", i);
		
		BGNext = new CBmpList;
		BGActual->next = BGNext;
		BGActual = BGNext;
		BGActual->Load(buffer);
	}
	BGActual->next = BGFirst;
	BGActual = BGFirst;

	OptActual = new CBmpList;

	OptFirst = OptActual;
	OptActual->Load("C:\\GameArt\\Menu\\Options\\opt0.bmp");
	for (i = 1; i <=29; i++)
	{
		char buffer[256];
		sprintf(buffer, "C:\\GameArt\\Menu\\Options\\opt%d.bmp", i);
		
		OptNext = new CBmpList;
		OptActual->next = OptNext;
		OptActual = OptNext;
		OptActual->Load(buffer);
	}
	OptActual->next = OptFirst;
	OptActual = OptFirst;

}

void InsertImage()
{
	RECT DestRect;

	for (int y = -480; y<=0; y+=2)
	{
		SetRect(&DestRect,0, y, 640, y+480);
		if (y < (-470))
			pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);
		pDDBackBuffer->Blt(&DestRect, pDDOffscreen2, NULL, DDBLT_WAIT, NULL);
		pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	}
	Sleep(2000);
}

BOOL BeginGame(HWND hwnd)
{
	if (INIT_ERROR) return FALSE;
	if (!DirectInputInit(hwnd))
	{
		MessageBox(hwnd,"Fatal error occured. The game cannot continue.","Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	if (!DirectDrawInit(hwnd))
	{
		MessageBox(hwnd,"Fatal error occured. The game cannot continue.","Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	
	BOOL b;
	b = Load.Load("c:\\GameArt\\Loading.bmp");
	if (!b) PostQuitMessage(0);

	Loading(LBT_BLACK, LM_STATIC);
	LoadMenuFiles();

	Engine.Load(0);

	SetTimer(hwnd, 1, 550, (TIMERPROC) Actualizare);	//18fps
	SetTimer(hwnd, 2, 55, (TIMERPROC) Actualizare);		//35fps
	SetTimer(hwnd, 3, 55, (TIMERPROC) Actualizare);		//53fps
	SetTimer(hwnd, 4, 55, (TIMERPROC) Actualizare);		//65fps
	SetTimer(hwnd, 5, 55, (TIMERPROC) Actualizare);		//67fps
	SetTimer(hwnd, 6, 55, (TIMERPROC) Actualizare);		//73fps
	return TRUE;
}

void UpdateMainMenu()
{
	static Selected = 0, SelOld, contor = 0, c = 0;
	static CurX = 320, CurY = 160, CurXAnte = 320, CurYAnte = 160;
	
	DIMOUSESTATE dims;
	HRESULT hr;
	HDC hdc;
	BOOL b, InRect, MEnter = FALSE, mGo = FALSE;
	int max = 3;

	hr = pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
	if (FAILED(hr)) PostQuitMessage(0);
	
	CurX+=dims.lX * MouseSensitiveness / 100;
	CurY+=dims.lY * MouseSensitiveness / 100;

	hr = pDDOffscreen->GetDC(&hdc);
	if (FAILED(hr)) PostQuitMessage(0);
	b = MenuBack.Draw(hdc);
	if (!b) PostQuitMessage(0);
	SetTextAlign(hdc, TA_BOTTOM | TA_RIGHT);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255,255,255));
	TextOut(hdc, 640, 480, VERSION, strlen(VERSION));
	hr = pDDOffscreen->ReleaseDC(hdc);
	if (FAILED(hr)) PostQuitMessage(0);
	hr = pDDBackBuffer->Blt(NULL, pDDOffscreen, NULL, DDBLT_WAIT, NULL);
	if (FAILED(hr)) PostQuitMessage(0); 

	POINT pCursor = {CurX, CurY};
	RECT DestRect, rBeginGame, rOptions;

	SetRect(&rBeginGame, 40, 170, 40 + 220, 170 + 220);
	SetRect(&rOptions, 360, 200, 360+170, 200 + 150);

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
	}
	
	if (PtInRect(&rBeginGame, pCursor)) InRect = TRUE;
	if (PtInRect(&rOptions, pCursor)) InRect = TRUE;

	if ((Selected == 1)&& (InRect))
	{
		c++;
		if (c % 2 == 0)
			BGActual = BGActual->next;
	}

	if ((Selected == 2)&& (InRect))
		OptActual = OptActual->next;

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

	SetRect(&DestRect, CurX, CurY, CurX + 32, CurY + 32);
	hr = pDDBackBuffer->Blt(&DestRect, pDDCursor, NULL, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	if (FAILED(hr)) PostQuitMessage(0);

	hr = pDDPrimary->Flip(NULL, DDFLIP_WAIT);
	if (FAILED(hr)) PostQuitMessage(0);
	
	if (dims.rgbButtons[0] & 0x80) MEnter = TRUE;
	if ((Selected == 1) && MEnter) State = GAME_ACTIVE;
}

void CALLBACK Actualizare(HWND hwnd, UINT iMsg, UINT iTimerID, DWORD dwTime)
{
	switch (State)
	{
		case GAME_ACTIVE:
			while (Engine.Update());
			PostQuitMessage(0);
			break;
		case MAIN_MENU:
			UpdateMainMenu();
			break;
	}
}
