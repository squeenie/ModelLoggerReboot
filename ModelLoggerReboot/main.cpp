#include <iostream>
#include <Windows.h>
#include <fstream>
#include <math.h>
#include <time.h>
#include "detours.h"
#include <Psapi.h>
#include <TlHelp32.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "DXFuncs.h"
#include "cModel.h"
#include <cMenu.h>
#include "ModelDump.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "Psapi.lib")
#define HOOK(func,addy) o##func = (t##func)DetourFunction((PBYTE)addy,(PBYTE)m##func)
 


typedef HRESULT (WINAPI* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
//typedef HRESULT (WINAPI* tSetStreamSource)(UINT StreamNumber,IDirect3DVertexBuffer9 *pStreamData,UINT OffsetInBytes,UINT Stride);
typedef HRESULT (WINAPI* tDrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount);

tEndScene oEndScene;
//tSetStreamSource oSetStreamSource;
tDrawIndexedPrimitive oDrawIndexedPrimitive;
 
using namespace std;
 
bool IsFavEntryUnique(cModel &entry);

RECT m_Rect;
LPDIRECT3DTEXTURE9 Red = NULL;
LPDIRECT3DTEXTURE9 Yellow = NULL;
LPDIRECT3DTEXTURE9 Green = NULL;
LPDIRECT3DTEXTURE9 Blue = NULL;
LPDIRECT3DTEXTURE9 Purple = NULL;
int iSelection = 0;
 
bool bPopList = false, bDrawElements = false, bAddNew = false, bRefresh = false; //logic bools
char b[32];
string strTmp;
UINT uiLastStride = 0;
bool bdip = true;

//test loading fin
bool bDrawItemsInFIN = false;
int iNumItemsinFIN = 0;
int iFINSelection = 0;
cModel DrawList[10000];
bool bShowError = true;
bool bUseFavList = false;
string dbErrorString;
//////

//test mouse manager

HRESULT WINAPI mEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	//MessageBox(NULL, "Entering endscene", "NO ERROR", MB_OK);
	if(dx_Font == NULL)
	{
		D3DXCreateFont(pDevice, 15, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana", &dx_Font);
	}
	if(AppManager == NULL)
	{
		//Logger.Init("\\logs\\lastrun.txt");
		//Logger.WriteLog("Initializing AppManager");
		AppManager = new cAppManager;
		AppManager->Initialize();
		if(AppManager == NULL)
		{
			//Logger.WriteLog("Failed");
			MessageBox(NULL, "AppManager failed to Init", "ERROR", MB_OK);
		} 
		if (Red == NULL)
			D3DXCreateTextureFromFileInMemory(pDevice, (LPCVOID)&bRed, sizeof(bRed), &Red);
		if (Blue == NULL)
			D3DXCreateTextureFromFileInMemory(pDevice, (LPCVOID)&bBlue, sizeof(bBlue), &Blue);
		if (Green == NULL)
			D3DXCreateTextureFromFileInMemory(pDevice, (LPCVOID)&bGreen, sizeof(bGreen), &Green);
		if (Purple == NULL)
			D3DXCreateTextureFromFileInMemory(pDevice, (LPCVOID)&bPurple, sizeof(bPurple), &Purple);
	}
	//#DEBUG_FOLLOW_PATH
	//AppManager->MsgBox("Endscene");
	if (ptrLine == NULL)
		D3DXCreateLine(pDevice, &ptrLine);
	//#DEBUG_FOLLOW_PATH
	//AppManager->MsgBox("Created Line");
	//#DEBUG_FOLLOW_PATH
	//AppManager->MsgBox("Updating Mouse");
	MouseManager.Update();
	//#DEBUG_FOLLOW_PATH
	//AppManager->MsgBox("Updated Mouse");
	if (AppManager->TestForm.bDraw)
	{
		//#DEBUG_FOLLOW_PATH
		//AppManager->MsgBox("Drawing Test Form");
		AppManager->TestForm.DrawTestForm(MouseManager, AppManager->iNumFavModels);
		if (AppManager->TestForm.pButtonList->isClicked(&MouseManager))
		{
			if (!bDrawItemsInFIN)
			{
				bDrawItemsInFIN = true;
			}
			else
			{
				if (AppManager->iNumFavModels == 0)
				{
					memcpy(&AppManager->FavList[AppManager->iNumFavModels], &AppManager->DrawList[iFINSelection], sizeof(cModel));
					AppManager->iNumFavModels++;
				}
				else if (AppManager->iNumFavModels < 100)
				{
					if ((IsFavEntryUnique(AppManager->DrawList[iFINSelection])))
					{
						memcpy(&AppManager->FavList[AppManager->iNumFavModels], &AppManager->DrawList[iFINSelection], sizeof(cModel));
						AppManager->iNumFavModels++;
					}
				}
			}
		}
	}

    m_Rect.top = 210;
	if (bDrawItemsInFIN)
	{
		if (bUseFavList)
		{
			strcpy(b, "Drawing from favourites list");
			dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
			m_Rect.top += 15;
		}
		else
		{
			strcpy(b, "Drawing from FIN list");
			dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
			m_Rect.top += 15;
		}
		

		strTmp.clear();
		strTmp = "Num items in FIN: ";
		strTmp += itoa(iNumItemsinFIN, b, 10);
		dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 255, 255));
		m_Rect.top += 15;

		strTmp.clear();
		strTmp = "FIN selection: ";
		strTmp += itoa(iFINSelection + 1, b, 10);
		dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 255, 255));
		m_Rect.top += 15;

		strTmp.clear();
		strTmp = "NumVertices: ";
		strTmp += itoa(AppManager->DrawList[iFINSelection].NumVertices, b, 10);
		dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 255, 255));
		m_Rect.top += 15;

		strTmp.clear();
		strTmp = "PrimCount: ";
		strTmp += itoa(AppManager->DrawList[iFINSelection].primCount, b, 10);
		dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 255, 255));
		m_Rect.top += 15;

		strTmp.clear();
		strTmp = "Stride: ";
		strTmp += itoa(AppManager->DrawList[iFINSelection].stride, b, 10);
		dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 255, 255));
		m_Rect.top += 15;

		strTmp.clear();
		strTmp = "FinFiles:";
		dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 255, 255));
		m_Rect.top += 15;
		for (int i = 0; i < AppManager->iNumFins; ++i)
		{
			m_Rect.top += 15;
			strcpy(b, AppManager->szFinList[i].c_str());
			dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(0, 255, 0));
		}
	}

	//#DEBUG_FOLLOW_PATH
	//AppManager->MsgBox("Printing Mouse Coords");

	//MouseStuff
	m_Rect.top += 15;
	strTmp = "Mouse XY: ";
	strTmp += itoa(MouseManager.CurrentPos.x, b, 10);
	strTmp += ", ";
	strTmp += itoa(MouseManager.CurrentPos.y, b, 10);
	dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(0, 255, 0));
	//

	//#DEBUG_FOLLOW_PATH
	//AppManager->MsgBox("Printing Hook status");

	m_Rect.top += 15;
	strcpy(b, "Status of Hooks:");
	dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(0, 0, 255));
	m_Rect.top += 15;
	strcpy(b, "Endscene");
	dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(0, 255, 0));
	m_Rect.top += 15;
	strcpy(b, "DrawIndexedPrimitive");
	if(bdip)
	{
		dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(0, 255, 0));
	}
	else
	{
		dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 00));
	}

	if(GetAsyncKeyState(VK_INSERT)&1)
	{
		bPopList = !bPopList;
	}
	if (GetAsyncKeyState(VK_DELETE)&1)
	{
		bDrawItemsInFIN = !bDrawItemsInFIN;
		if (bDrawItemsInFIN)
		{
			iNumItemsinFIN = AppManager->LoadFinFile();
			if (AppManager->FindFinFiles() <= 0)
			{
				MessageBox(NULL, "No fins", ":(", MB_OK);
			}

		}
	}
	if (GetAsyncKeyState(VK_HOME) & 1)
	{
		AppManager->TestForm.bDraw = !AppManager->TestForm.bDraw;
	}

	if (GetAsyncKeyState(VK_UP) & 1)
	{
		iFINSelection++;
		//AppManager->TestForm.iHeight-= 3;
	}
	if (GetAsyncKeyState(VK_DOWN) & 1)
	{
		iFINSelection--;
		//AppManager->TestForm.iHeight+=3;
	}
	if (GetAsyncKeyState(VK_END) & 1)
	{
		iFINSelection = 0;
	}
	if (GetAsyncKeyState(VK_LEFT) & 1)
	{
		iFINSelection -= 100;
		//AppManager->TestForm.iWidth-=3;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 1)
	{
		iFINSelection += 100;
		//AppManager->TestForm.iWidth+=3;
	}
	if (GetAsyncKeyState(VK_BACK))
	{
		bUseFavList = !bUseFavList;
		iFINSelection = 0;
	}

	if (iFINSelection < 0)
	{
		iFINSelection = 0;
	}
	if (iFINSelection >= iNumItemsinFIN)
	{
		iFINSelection = iNumItemsinFIN - 1;
	}

	if(bPopList)
	{ 
		m_Rect.top += 15;
		strcpy(b, "Dumping");
		dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
	}


	m_Rect.top += 15;
	
	if (bShowError)
	{
		dx_Font->DrawTextA(NULL, dbErrorString.c_str(), dbErrorString.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 00));
	}

    return oEndScene(pDevice);
}

 
HRESULT WINAPI mDrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
	//MessageBox(NULL, "In DIP", "NO ERROR", MB_OK);
	bdip = true;
	//Logger.WriteLog("Calling GetStreamSource");
	IDirect3DVertexBuffer9* vertStreamData;
    UINT uOffsetInBytes;
    UINT uStride;
    pDevice->GetStreamSource(0,&vertStreamData,&uOffsetInBytes,&uStride);
	//Logger.WriteLog("Done");
	if(bPopList)
	{
		AppManager->ReadInfoFile();
		AppManager->bIsDumping = true;
		AppManager->bWasDumping = true;
		//Logger.WriteLog("Populating List...");
		if(AppManager->CurrentChunk->iNumItems >= MODELS_PER_CHUNK)
		{
			//MessageBox(NULL, "Dumping Chunk", "NO ERROR", MB_OK);
			//Logger.WriteLog("Too many items! Dumping chunk!");
			AppManager->DumpChunk();
		}
		//Logger.WriteLog("Adding Model to chunk...");
		AppManager->CurrentChunk->cList[AppManager->CurrentChunk->iNumItems].NumVertices = NumVertices;
		AppManager->CurrentChunk->cList[AppManager->CurrentChunk->iNumItems].primCount = primCount;
		AppManager->CurrentChunk->cList[AppManager->CurrentChunk->iNumItems].stride = uStride;
		AppManager->CurrentChunk->iNumItems++;
	}
	else
	{
		AppManager->bIsDumping = false;
		if(AppManager->CurrentChunk->iNumItems != 0)
		{
			//Logger.WriteLog("Logging stopped, dumping last chunk.");
			AppManager->DumpChunk();
		}
		if(!AppManager->bIsDumping && AppManager->bWasDumping)
		{
			
			AppManager->iTotalDumps += 1 ;
			AppManager->WriteInfoFile();
			AppManager->bWasDumping = false;
		}
	}
	if (bDrawItemsInFIN)
	{
		if (!bUseFavList)
		{
			if (AppManager->DrawList[iFINSelection].NumVertices == NumVertices &&
				AppManager->DrawList[iFINSelection].primCount == primCount &&
				AppManager->DrawList[iFINSelection].stride == uStride && uStride == 32)//remove last bit later, this is just to stop text flickering. Filters will be added later to deal with this crap.
			{
				pDevice->SetRenderState(D3DRS_ZENABLE, false);
				pDevice->SetTexture(0, Green);
				oDrawIndexedPrimitive(pDevice, PrimType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
				pDevice->SetRenderState(D3DRS_ZENABLE, true);
				pDevice->SetTexture(0, Blue);
			}
		}
		else
		{
			if (AppManager->iNumFavModels > 0)
			{
				if (AppManager->FavList[iFINSelection].NumVertices == NumVertices &&
					AppManager->FavList[iFINSelection].primCount == primCount &&
					AppManager->FavList[iFINSelection].stride == uStride /*&& uStride == 32*/)	//remove last bit later, this is just to stop text flickering. Filters will be added later to deal with this crap.
				{
					pDevice->SetRenderState(D3DRS_ZENABLE, false);
					pDevice->SetTexture(0, Green);
					oDrawIndexedPrimitive(pDevice, PrimType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
					pDevice->SetRenderState(D3DRS_ZENABLE, true);
					pDevice->SetTexture(0, Blue);
				}
			}
		}
	}
 
    return oDrawIndexedPrimitive(pDevice, PrimType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

void SetHooks()
{
	//MessageBox(NULL, "Set Hooks", "NO ERROR", MB_OK);
    DWORD table[3] = {0};
 
    DX_Init(table);

	//MessageBox(NULL, "Init DX", "NO ERROR", MB_OK);
    HOOK(EndScene,table[ES]);
	Sleep(400);
	//MessageBox(NULL, "Hook Endscene", "NO ERROR", MB_OK);
    HOOK(DrawIndexedPrimitive,table[DIP]);
	//MessageBox(NULL, "Hook DIP", "NO ERROR", MB_OK);
}
 
int WINAPI DllThread()
{   
    SetHooks();
    //AllocConsole();
    return 0;
}
 
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD Reason, LPVOID lpReserved)
{
    switch(Reason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) DllThread, NULL, NULL, NULL);
        break;
    }
    return true;
}			

bool IsFavEntryUnique(cModel &entry) //Eventually put this in the logger and remove dependancy on the sorter? (potentially...hash tables will make it a whole lot faster)
{
	bool bUnique = false;
	if (AppManager->iNumFavModels == 0)
	{
		return true;
	}
	for (int iListIndex = 0; iListIndex < AppManager->iNumFavModels; ++iListIndex)
	{
		if (AppManager->FavList[iListIndex].NumVertices == entry.NumVertices &&
			AppManager->FavList[iListIndex].primCount == entry.primCount &&
			AppManager->FavList[iListIndex].stride == entry.stride)
		{
			bUnique = false;
			break;
		}
		else
		{
			bUnique = true;
		}
	}
	return bUnique;
}