// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <sstream>
#include <fstream>
#include <regex>
#include <filesystem>

#include <fmt/xchar.h>

#include "Version.h"
#include "framework.h"

#define WM_UPDATE_PROGRESS (WM_USER + 301)
#define WM_END_LOAD_PLAYLIST (WM_USER + 302)
#define WM_UPDATE_PROGRESS_STREAM (WM_USER + 303)
#define WM_END_GET_STREAM_INFO (WM_USER + 304)

#endif //PCH_H
