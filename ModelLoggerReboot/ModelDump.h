#ifndef _MODELDUMP_H_
#define _MODELDUMP_H_

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <ctime>
#include <string>
#include "cModel.h"
#include <cMenu.h>
#include <dirent.h>
//#include <logger.h>

//TODO:
//Implement FavList + a slightly nicer UI and we're set for Alpha release!

//cLogger Logger;
char debug[256];
const int MODELS_PER_CHUNK = 250;
BYTE CurrentVersion = 0x00;
BYTE CurrentImplementation = 0x01;
typedef unsigned char BYTE;

using namespace std;

struct sModelDumpFilev1
{
	BYTE xVersion;// eg 0x02 and
	BYTE xImplementation;// 0x01 is verson 2.01
	int iNumEntries; //incase its not a full chunmk
	cModel cEntries[MODELS_PER_CHUNK];
	static const DWORD dwEOF = 0xDEADBEEF; //I like beef
};

struct sInfoFile //This will be used to keep track of seperate dumps, note which have been analyzed etc. We'll work it out on the fly cause that always works right
{
	string szGameName;
	int iNumDumps; //Work the rest out later, this was basically a solution to getting unique file names for different dumps but the idea became a bit more organic
};

struct sChunk //one chunk holds a list of models to be logged. Pretty simple for the time being
{
	int iNumItems;
	cModel cList[MODELS_PER_CHUNK];
	void ClearChunk()
	{
		iNumItems = 0;
		ZeroMemory(cList, sizeof(cModel) * MODELS_PER_CHUNK);
	}
};

class cAppManager
{
public:
	ofstream DumpFileOut;
	ifstream InfoFileIn;
	ofstream InfoFileOut;

	ifstream FileIn; // to replace redundant things
	ofstream FileOut;
	
	bool bIsDumping;
	bool bWasDumping;
	int iTotalChunks;
	int iTotalDumps;
	sChunk *CurrentChunk;
	sModelDumpFilev1 *DumpFile;
	string szFileName;
	string szGameName;
	void Initialize();
	void Cleanup();
	void DumpChunk();
	void WriteInfoFile();
	void ReadInfoFile();
	void CreateInfoFile();
	void MsgBox(char *Message);

	//New test stuff ie not thought out properly yet
	cModel *DrawList;
	cModel *TempList;
	cModel FavList[1000]; // Change to dynamic
	cModel FavList2[1000]; // Change to dynamic
	ifstream FinFileIn;
	cForm TestForm;
	bool LoadFile(string szFileName, int *TotalEntries, cModel *List);
	bool SaveFile(string szFileName, int TotalEntries, cModel *List);
	int LoadFinFile();
	int FindFinFiles();
	int iNumFins;
	int iNumFavModels;
	string szFinList[32]; //fix this
	bool bFinListInitialized;
	ID3DXLine *ptrLine;
};
/*
cAppManager is used to handle everything. At the moment it is very basic but it has a lot of room for expansion.
<TODO>
*Add notes/documentation etc
*/
void cAppManager::Initialize()
{
	this->DrawList = NULL;
	this->CurrentChunk = new sChunk;
	this->DumpFile = new sModelDumpFilev1;
	this->bIsDumping = false;
	this->bWasDumping = false;
	this->CurrentChunk->ClearChunk();
	this->iTotalChunks = 0;
	this->iTotalDumps = 0;
	this->ReadInfoFile();
	//this->szFinList = NULL;
	this->iNumFins = 0;
	this->iNumFavModels = 0;
	this->bFinListInitialized = false;
	

	//test stuff
	this->TestForm.x = 500;
	this->TestForm.y = 50;
	this->TestForm.bDraw = true;
	this->TestForm.iHeight = 200;
	this->TestForm.iWidth = 200;
	
	this->TestForm.r = 110;
	this->TestForm.g = 110;
	this->TestForm.b = 110;
	this->TestForm.a = 125;
	this->TestForm.szFormText = "Favourites";
	this->TestForm.szFormTitle = "TestForm";
	this->TestForm.bClicked = false;
	this->TestForm.bMouseStilDown = false;

	this->TestForm.pButtonList = new cButton;
	this->TestForm.pButtonList->szButtonText = "Add";
	this->TestForm.pButtonList->szButtonName = "TestButton";
	this->TestForm.pButtonList->iHeight = 32;
	this->TestForm.pButtonList->iWidth = 64;
	this->TestForm.pButtonList->x = this->TestForm.x + 50;
	this->TestForm.pButtonList->y = this->TestForm.y + 75;

	this->TestForm.pCloseBox = new cCheckBox;
	this->TestForm.pCloseBox->width = 10;
	this->TestForm.pCloseBox->height = 10;
	this->TestForm.pCloseBox->r = 0;
	this->TestForm.pCloseBox->g = 0;
	this->TestForm.pCloseBox->b = 255;
	this->TestForm.pCloseBox->x = this->TestForm.x + (this->TestForm.iWidth - 4) - this->TestForm.pCloseBox->width - 4;
	this->TestForm.pCloseBox->y = this->TestForm.y + 4;
}

void cAppManager::DumpChunk()
{
	//Logger.WriteLog("Called DumpChunk()");
	char tmp[128];
	ZeroMemory(tmp, 128);
	szFileName = "dumps\\";
	for (int i = 0; i < 4; ++i)
	{
		szFileName += szGameName[i];
		if (szGameName[i] == '\0')
			break;
	}
	szFileName += itoa(iTotalDumps, tmp, 10);
	szFileName += "-";
	szFileName += itoa(iTotalChunks, tmp, 10);
	szFileName += ".mdldmp";
	//MessageBox(NULL, szFileName.c_str(), "DUMP", MB_OK);
	DumpFileOut.open(szFileName.c_str());
	DumpFile->xVersion = CurrentVersion;
	DumpFile->xImplementation = CurrentImplementation;
	DumpFile->iNumEntries = CurrentChunk->iNumItems;
	//Logger.WriteLog("Dumping...");
	//start dumping contents
	DumpFileOut << DumpFile->xVersion << endl;
	DumpFileOut << DumpFile->xImplementation << endl;
	DumpFileOut << DumpFile->iNumEntries << endl;
	for(int i = 0; i < DumpFile->iNumEntries; ++i)
	{
		DumpFileOut << CurrentChunk->cList[i].NumVertices << endl;
		DumpFileOut << CurrentChunk->cList[i].primCount << endl;
		DumpFileOut << CurrentChunk->cList[i].stride<< endl;
	}
	DumpFileOut << DumpFile->dwEOF;
	CurrentChunk->ClearChunk();
	iTotalChunks++;
	DumpFileOut.close();
}

void cAppManager::CreateInfoFile()
{
	ofstream tmp;
	tmp.open("dumps\\info.dat");
	szGameName = "rfgi";
	tmp << szGameName << endl; //figure out something else later, too tired right now
	tmp << 0;
	tmp.close();
}

void cAppManager::ReadInfoFile()
{
	//MessageBox(NULL, "ReadInfoFile", "DEBUG", MB_OK);
	InfoFileIn.open("dumps\\info.dat");
	if (!InfoFileIn)
	{
		InfoFileIn.close();
		//MessageBox(NULL, "Not found, creating InfoFile", "DEBUG", MB_OK);
		CreateInfoFile();
	}
	else
	{
		string tmp;
		
		getline(InfoFileIn, szGameName);
		getline(InfoFileIn, tmp);
		iTotalDumps = atoi(tmp.c_str());
		//MessageBox(NULL, itoa(iTotalDumps, debug, 10), "DEBUG", MB_OK);
	}
	InfoFileIn.close();
}

void cAppManager::WriteInfoFile()
{
	//MessageBox(NULL, "WriteInfoFile()", "DONE", MB_OK);
	InfoFileOut.open("dumps\\info.dat");
	InfoFileOut << szGameName << endl;
	InfoFileOut << iTotalDumps << endl;
	InfoFileOut.close();
	iTotalChunks = 0;
}

void cAppManager::Cleanup()
{
	delete DumpFile;
	delete CurrentChunk;
}

bool cAppManager::LoadFile(string szFileName, int *TotalEntries, cModel *List) //To replace redundant functions
{
	int size = 0;
	this->FileIn.open("dumps\\" + szFileName);
	if (this->FileIn)
	{
		this->FileIn >> size;
	/*	if (FavList == NULL)
		{
			FavList = new cModel[size * 2];
		}*/
		//else
		//{
		//	//delete List;
		//	List = new cModel[size];
		//}
		for (int i = 0; i < size; ++i)
		{
			this->FileIn >> FavList[i].NumVertices;
			this->FileIn >> FavList[i].primCount;
			this->FileIn >> FavList[i].stride;
			//*TotalEntries++;
			this->iNumFavModels++;
		}

		this->FileIn.close();

		if (List != NULL)
		{
			return true;
		}
		else
		{
			MessageBox(NULL, "List NULL", "SadFace", MB_OK);
			return false;
		}
	}

	else
	{
		return false;
	}
}

bool cAppManager::SaveFile(string szFileName, int TotalEntries, cModel *List) //To replace LoadFin, LoadDump etc
{
	int size = 0; // TODO class up FIN stuff
	szFileName = "dumps\\" + szFileName;
	this->FileOut.open(szFileName);
	if (this->FileOut)
	{
		FileOut << TotalEntries << endl;

		for (int i = 0; i < TotalEntries; ++i)
		{
			FileOut << List[i].NumVertices << endl;
			FileOut << List[i].primCount << endl;
			FileOut << List[i].stride << endl;
		}

		FileOut.close();
		return true;
	}

	else
	{
		return false;
	}
}

int cAppManager::LoadFinFile() //to be replaced with loadfile
{
	int size = 0; // TODO class up FIN stuff
	this->FinFileIn.open("dumps\\test.fin");
	if (this->FinFileIn)
	{
		this->FinFileIn >> size;
		if (this->DrawList == NULL) //if its fresh
		{
			this->DrawList = new cModel[size];
		}
		else //get rid of it, start anew
		{
			delete this->DrawList;
			this->DrawList = new cModel[size];
		}
		for (int i = 0; i < size; ++i)
		{
			this->FinFileIn >> DrawList[i].NumVertices;
			this->FinFileIn >> DrawList[i].primCount;
			this->FinFileIn >> DrawList[i].stride;
		}

		this->FinFileIn.close();

		if (this->DrawList != NULL)
		{
			this->bFinListInitialized = true;
			return size;
		}
		else
		{
			MessageBox(NULL, "DrawList NULL", "SadFace", MB_OK);
			return -1;
		}
	}
	
	else
	{
		return -1;
	}
}

int cAppManager::FindFinFiles()
{
	this->iNumFins = 0;
	char tmp[250];
	char *extPtr = NULL;
	int tmpint = 0;
	GetCurrentDirectory(250, tmp);
	strcat(tmp, "\\dumps");
	
	DIR *dir = opendir(tmp);
	if (!dir)
	{
		return -1;
	}

	dirent *entry;
	while (entry = readdir(dir))
	{
		if (this->iNumFins < 16)
		{
			extPtr = strstr(entry->d_name, ".fin");
			if (extPtr)
			{
				this->szFinList[this->iNumFins] = entry->d_name;
				this->iNumFins++;
			}
		}
	}

	closedir(dir);
	return this->iNumFins;
}

void cAppManager::MsgBox(char *Message)
{
	MessageBox(NULL, Message, "DEBUG", MB_OK);
}

cAppManager *AppManager = NULL;

#endif