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

#else // _DEBUG�A����Release��

#define WriteDebugConsoleT Debug_ReleaseFunction

#endif // _DEBUG

void _InitializeDebugFeature();
void _TerminateDebugFeature();

void _WriteDebugConsoleA(LPCSTR string);
void _WriteDebugConsoleW(LPCWSTR string);
void Debug_ReleaseFunction(...);