#pragma once

#include <Windows.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <ios>

#include "Config.h"

#ifdef _DEBUG
#ifdef UNICODE

#define WriteDebugConsole _WriteDebugConsoleW

#else // UNICODE�A���̓}���`�o�C�g��

#define WriteDebugConsole _WriteDebugConsoleA

#endif // UNICODE

#define InitializeDebugFeature _InitializeDebugFeature
#define TerminateDebugFeature _TerminateDebugFeature
#define WriteDebugConsoleU _WriteDebugConsoleU

#else // _DEBUG�A����Release��

#define WriteDebugConsole Debug_ReleaseFunction
#define InitializeDebugFeature Debug_ReleaseFunction
#define TerminateDebugFeature Debug_ReleaseFunction
#define WriteDebugConsoleU Debug_ReleaseFunction

#endif // _DEBUG

void _InitializeDebugFeature();
void _TerminateDebugFeature();

void _WriteDebugConsoleA(LPCSTR string);
void _WriteDebugConsoleW(LPCWSTR string);
void _WriteDebugConsoleU(const std::string& message);
void Debug_ReleaseFunction(...);