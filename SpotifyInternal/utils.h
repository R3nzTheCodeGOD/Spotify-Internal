#ifndef UTILS_H
#define UTILS_H
#pragma once

#include <Windows.h>
#include <iostream>


typedef struct
{
	DWORD dwMajor;
	DWORD dwMinor;
	DWORD dwBuild;
	DWORD dwRevision;
} version_t;

namespace utils
{
	bool get_version(version_t* v);

}

#endif
