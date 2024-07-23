// MWExtract.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Esm\EsmFile.h"
#include "Esm\EsmWinUtils.h"
#include <unordered_map>
#include <string>

typedef std::unordered_map<std::string, int> refmap_t;


bool ParseCellReferences(CEsmCell* pCell, CGenFile& csvFile)
{
	int iCellRef;
	int refCount = 0;
	CEsmSubCellRef* pCellRef;
	refmap_t refMap;
	char buffer[1024];

	if (pCell == nullptr) return false;

	printf("\tFound Cell %s in region %s at (%d, %d) Flags %02X\n", pCell->GetName(), pCell->GetRegion(), pCell->GetGridX(), pCell->GetGridY(), pCell->GetFlags());
	
	pCellRef = dynamic_cast<CEsmSubCellRef *>(pCell->FindFirst(MWESM_SUBREC_CREF, iCellRef));
	
	while (pCellRef != nullptr)
	{
		++refCount;

		int refId = pCellRef->GetIndex();
		CEsmSubName* pName = dynamic_cast<CEsmSubName *>(pCellRef->FindSubRecord(MWESM_SUBREC_NAME));

		if (pName)
			_snprintf(buffer, 500, "%s", pName->GetName());
		else
			_snprintf(buffer, 500, "0x%08X", refId);

		refMap[buffer] += 1;
		pCellRef = dynamic_cast<CEsmSubCellRef *>(pCell->FindNext(MWESM_SUBREC_CREF, iCellRef));
	}

	char cellName[256];

	if (pCell->IsInterior())
		_snprintf(cellName, 200, "%s", pCell->GetName());
	else
		_snprintf(cellName, 200, "%s(%d,%d)", pCell->GetName(), pCell->GetGridX(), pCell->GetGridY());

	printf("\t\tFound %d cell references (%d unique)\n", refCount, refMap.size());

	for (auto i : refMap)
	{
		csvFile.Printf("\"%s\",\"%s\",\"%s\",%d\n", i.first.c_str(), pCell->IsInterior() ? "Interior" : "World", cellName, i.second);
	}

	return true;
}


bool ExportItemUses(const char* pEsmFilename, const char* pCsvFilename)
{
	CEsmFile EsmFile;
	CGenFile csvFile;

	if (!csvFile.Open(pCsvFilename, "wb"))
	{
		printf("Error: Failed to open CSV file %s for output!\n", pCsvFilename);
		return false;
	}

	csvFile.Printf("Reference, Cell Type, Cell Name, Count\n");

	printf("Loading %s...\n", pEsmFilename);

	if (!EsmFile.Read(pEsmFilename))
	{
		printf("Error: Failed to load %s!\n", pEsmFilename);
		return false;
	}

	printf("Loaded Morrowind.esm with %d records!\n", EsmFile.GetNumRecords());

	CEsmRecArray& Records = *EsmFile.GetRecords();
	int cellCount = 0;

	for (int i = 0; i < Records.GetSize(); ++i)
	{
		auto pRecord = Records.GetAt(i);
		refmap_t refMap;

		if (pRecord->IsType(MWESM_REC_CELL))
		{
			ParseCellReferences(dynamic_cast<CEsmCell*>(pRecord), csvFile);
			++cellCount;
		}
	}

	return true;
}


int main()
{
	

	//FindMWRegistryPath();
	//printf("Morrowind Installation Folder: %s\n", GetMWDataPath());
	//SetMWDataPath("D:\\Morrowind\\");
	SetMWDataPath("D:\\SteamLibrary\\steamapps\\common\\Morrowind\\Data Files\\");

	ExportItemUses("D:\\SteamLibrary\\steamapps\\common\\Morrowind\\Data Files\\Morrowind.esm", "mwitemuse.csv");
	ExportItemUses("D:\\SteamLibrary\\steamapps\\common\\Morrowind\\Data Files\\Bloodmoon.esm", "mw-tr-itemuse.csv");
	ExportItemUses("D:\\SteamLibrary\\steamapps\\common\\Morrowind\\Data Files\\Tribunal.esm", "mw-bm-itemuse.csv");

	

    return 0;
}


