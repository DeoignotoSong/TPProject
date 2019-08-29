#pragma  once
#include "FileReader.h"

string curPath() {
	LPWSTR buffer= new wchar_t[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	wstring ws(buffer);
	string str = string(ws.begin(), ws.end());
	string::size_type pos = str.find_last_of("\\/");
	return str.substr(0, pos);
}

bool loadFile2Vector( string fileName,  vector<string>& vecOfStrs)
{
	cout << "my directory is " << curPath() << "\n";
	// Open the File
	ifstream in(fileName.c_str());

	// Check if object is valid
	if (!in)
	{
		 cerr << "Cannot open the File : " << fileName <<  endl;
		return false;
	}

	 string str;
	// Read the next line from File untill it reaches the end.
	while ( getline(in, str))
	{
		// Line contains string of length > 0 then save it in vector
		if (str.size() > 0)
			vecOfStrs.push_back(str);
	}
	//Close The File
	in.close();
	return true;
}
