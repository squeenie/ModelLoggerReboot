#include <iostream>
#include <fstream>
#include <Windows.h>
#include <ctime>
#include <string>
#include "cModel.h"
#include <logger.h>

//CURRENT TASKS:
//Write chunk to dump file, make it all work by 9:30pm
//Status: Done...at 11:46pm the next day

//NEXT TASK:
//Scan, sort and remove duplicates.

cLogger Logger;

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

struct sChunk //one chunk holds a list of models to be logged. Dynamically initialized but we're going to try to stick to around 500 entries, it could get a bit too big
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
	bool bIsDumping;
	bool bWasDumping;
	int iTotalChunks;
	sChunk *CurrentChunk;
	sModelDumpFilev1 *DumpFile;
	string FileName;
	void Initialize();
	void Cleanup();
	void DumpChunk();
	void UpdateInfoFile();
};
/*
cAppManager is used to handle everything. At the moment it is very basic but it has a lot of room for expansion.
*/
void cAppManager::Initialize()
{
	CurrentChunk = new sChunk;
	DumpFile = new sModelDumpFilev1;
	bIsDumping = false;
	bWasDumping = false;
	CurrentChunk->ClearChunk();
	iTotalChunks = 0;
}

void cAppManager::DumpChunk()
{
	//Logger.WriteLog("Called DumpChunk()");
	char tmp[128];
	ZeroMemory(tmp, 128);
	FileName = "dumps\\rfg_";
	FileName += itoa(iTotalChunks, tmp, 10);
	FileName += ".mdldmp";
	//MessageBox(NULL, FileName.c_str(), "DUMP", MB_OK);
	DumpFileOut.open(FileName.c_str());
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

void cAppManager::UpdateInfoFile()
{
	MessageBox(NULL, "TODO:UpdateInfoFile", "TODO", MB_OK);
}


void cAppManager::Cleanup()
{
	delete DumpFile;
	delete CurrentChunk;
}

cAppManager *AppManager = NULL;
