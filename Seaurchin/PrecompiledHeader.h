#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

//Windows
#include <Windows.h>
#include <Shlwapi.h>
#include <Imm.h>

//C Runtime
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cmath>

//C++ Standard
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <chrono>
#include <ios>
#include <map>
#include <utility>
#include <limits>
#include <unordered_map>
#include <forward_list>
#include <list>
#include <tuple>
#include <random>
#include <exception>
#include <future>
#include <thread>

//Boost
#include <boost/config.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/crc.hpp>
#include <boost/any.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/optional.hpp>
#include <boost/range/sub_range.hpp>

//Libraries
#include <DxLib.h>
//#include <EffekseerForDXLib.h>

#include <angelscript.h>
#include <scriptarray\scriptarray.h>
#include <scriptmath\scriptmath.h>
#include <scriptmath\scriptmathcomplex.h>
#include <scriptstdstring\scriptstdstring.h>
#include <scriptdictionary\scriptdictionary.h>
#include "wscriptbuilder.h"
//#include "as_smart_ptr_wrapper.h"

#include <WacomMultiTouch.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <zlib.h>
#include <png.h>

#include <bass.h>
#include <bassmix.h>
#include <bass_fx.h>

#include <msgpack.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/wincolor_sink.h>
#include <spdlog/sinks/sink.h>

#include <toml/toml.h>