#include <iostream>
#include <fstream>
#include <Windows.h>
#include <ctime>
#include <string>
#include "cModel.h"
#include <logger.h>

//CURRENT TASKS:
//Write chunk to dump file, make it all work by 9:30pm
//Status: Done...at 11:46pm the next day, should plan ahead (still have to figure out a decent naming scheme or something)

//NEXT TASK:
//Scan, sort and remove duplicates.

cLogger Logger;
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
};
/*
cAppManager is used to handle everything. At the moment it is very basic but it has a lot of room for expansion.
<TODO>
*Add notes
*/
void cAppManager::Initialize()
{
	CurrentChunk = new sChunk;
	DumpFile = new sModelDumpFilev1;
	bIsDumping = false;
	bWasDumping = false;
	CurrentChunk->ClearChunk();
	iTotalChunks = 0;
	iTotalDumps = 0;
	ReadInfoFile();
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
		//iTotalDumps = atoi(tmp.c_str());
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

cAppManager *AppManager = NULL;
