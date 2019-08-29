#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
using namespace std;

string curPath();
bool loadFile2Vector(string fileName, vector<string>& vecOfStrs);