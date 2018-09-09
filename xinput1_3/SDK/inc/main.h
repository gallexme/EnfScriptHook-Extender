#pragma once

#include <windows.h>

#define IMPORT __declspec(dllimport)

IMPORT void scriptRegister(char * name, INT64 func);