/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

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
#include <fmt/chrono.h>

#include "Version.h"
#include "framework.h"

#define WM_INIT_PROGRESS (WM_USER + 301)
#define WM_UPDATE_PROGRESS (WM_USER + 302)
#define WM_END_LOAD_PLAYLIST (WM_USER + 303)
#define WM_END_LOAD_JSON_PLAYLIST (WM_USER + 304)
#define WM_UPDATE_PROGRESS_STREAM (WM_USER + 305)
#define WM_END_GET_STREAM_INFO (WM_USER + 306)
#define WM_NOTIFY_END_EDIT (WM_USER + 307)
#define WM_SWITCH_PLUGIN (WM_USER + 308)
#define WM_ON_EXIT (WM_USER + 309)


#endif //PCH_H
