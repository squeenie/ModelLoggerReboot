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
 

 
RECT m_Rect;
LPDIRECT3DTEXTURE9 Red = NULL;
LPDIRECT3DTEXTURE9 Yellow = NULL;
LPDIRECT3DTEXTURE9 Green = NULL;
LPDIRECT3DTEXTURE9 Blue = NULL;
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
cModel DrawList[1000];
ifstream FinFile;
bool bShowError = true;
string dbErrorString;
//////
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
		if (Green == NULL)
			D3DXCreateTextureFromFileInMemory(pDevice, (LPCVOID)&bGreen, sizeof(bGreen), &Green);
	}

    m_Rect.top = 210;
	if (bDrawItemsInFIN)
	{
		strcpy(b, "Drawing!");
		dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
		m_Rect.top += 15;

		strTmp.clear();
		strTmp = "Num items in FIN: ";
		strTmp += itoa(iNumItemsinFIN, b, 10);
		dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
		m_Rect.top += 15;

		strTmp.clear();
		strTmp = "FIN selection: ";
		strTmp += itoa(iFINSelection + 1, b, 10);
		dx_Font->DrawTextA(NULL, strTmp.c_str(), strTmp.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
		m_Rect.top += 15;
	}
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
	}
	if (GetAsyncKeyState(VK_UP) & 1)
	{
		iFINSelection++;
	}
	if (GetAsyncKeyState(VK_DOWN) & 1)
	{
		iFINSelection--;
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
		dx_Font->DrawTextA(NULL, b, strlen(b), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 00));
	}

	if (bDrawItemsInFIN)
	{
		if (iNumItemsinFIN == 0)
		{
			FinFile.open("dumps\\rfgi.fin");
				if (!FinFile)
				{
					dbErrorString = "Cant load FIN";
				}
				else
				{
					FinFile >> iNumItemsinFIN;
					for (int i = 0; i < iNumItemsinFIN; ++i)
					{
						FinFile >> DrawList[i].NumVertices;
						FinFile >> DrawList[i].primCount;
						FinFile >> DrawList[i].stride;
					}
				}
		}
	}
	else
	{
		if (FinFile.is_open())
		{
			FinFile.close();
		}
	}

	m_Rect.top += 15;
	
	if (bShowError)
		dx_Font->DrawTextA(NULL, dbErrorString.c_str(), dbErrorString.length(), &m_Rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 00));

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
		if (DrawList[iFINSelection].NumVertices == NumVertices &&
			DrawList[iFINSelection].primCount == primCount &&
			DrawList[iFINSelection].stride == uStride)
			{
				pDevice->SetRenderState(D3DRS_ZENABLE, false);
				pDevice->SetTexture(0, Green);
				oDrawIndexedPrimitive(pDevice, PrimType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
				pDevice->SetRenderState(D3DRS_ZENABLE, true);
				pDevice->SetTexture(0, Red);
			}
	}
	/*pDevice->SetRenderState( D3DRS_ZENABLE,false );
    pDevice->SetTexture(0, Green);
    oDrawIndexedPrimitive(pDevice, PrimType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
    pDevice->SetRenderState( D3DRS_ZENABLE, true );
    pDevice->SetTexture(0, Red); */
 
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