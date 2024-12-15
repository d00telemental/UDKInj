#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>

#define LOG_INFO(format, ...)           ::wprintf(L"[INFO] " format L"\n", __VA_ARGS__)
#define LOG_ERROR(format, ...)          do { ::wprintf(L"[ERROR] " format L"\n", __VA_ARGS__); ::fflush(stdout); } while (false)

#define ASI_EXPORT extern "C" __declspec(dllexport) void
