#include "Misc.h"
#include "ScriptScene.h"

using namespace std;

/*
string ConvertUTF8ToShiftJis(const string &utf8str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, nullptr, 0);
    wchar_t *buffer = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, buffer, len);
    int sjis = WideCharToMultiByte(CP_ACP, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    char *sbuffer = new char[sjis];
    WideCharToMultiByte(CP_ACP, 0, buffer, -1, sbuffer, sjis, nullptr, nullptr);
    string ret = sbuffer;
    delete[] buffer;
    delete[] sbuffer;
    return ret;
}

string ConvertShiftJisToUTF8(const string &sjisstr)
{
    int len = MultiByteToWideChar(CP_ACP, 0, sjisstr.c_str(), -1, nullptr, 0);
    wchar_t *buffer = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, sjisstr.c_str(), -1, buffer, len);
    int utf8 = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    char *ubuffer = new char[utf8];
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ubuffer, utf8, nullptr, nullptr);
    string ret = ubuffer;
    delete[] buffer;
    delete[] ubuffer;
    return ret;
}
*/

wstring ConvertUTF8ToUnicode(const string &utf8str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, nullptr, 0);
    wchar_t *buffer = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, buffer, len);
    wstring ret = buffer;
    delete[] buffer;
    return ret;
}

string ConvertUnicodeToUTF8(const wstring &utf16str)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(utf16str.c_str()), -1, nullptr, 0, nullptr, nullptr);
    char *buffer = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(utf16str.c_str()), -1, buffer, len, nullptr, nullptr);
    string ret = buffer;
    delete[] buffer;
    return ret;
}

void ScriptSceneWarnOutOf(const string &type, asIScriptContext *ctx)
{
    const char *secn;
    int col, row;
    row = ctx->GetLineNumber(0, &col, &secn);
    ctx->GetEngine()->WriteMessage(secn, row, col, asEMsgType::asMSGTYPE_WARNING, ("You can call Yield Function only from " + type + "!").c_str());
}

double ToDouble(const char *str)
{
    double result = 0.0, base = 1.0;
    int sign = 1;
    const char *it = str;
    unsigned char ch = '0';

    it = *it == '-' ? (sign = -sign, ++it) : it;
    while (((unsigned char)(ch = *(it++) - '0')) <= 9) result = result * 10 + ch;
    if (*(--it) == '.') while (((unsigned char)(ch = *(++it) - '0')) <= 9) result += (base *= 0.1) * ch;
    return sign * result;
}

double NormalizedFmod(double x, double y)
{
    if (y < 0) y = -y;
    int q = x >= 0 ? x / y : (x / y) - 1;
    return x - q * y;
}

int32_t ConvertInteger(const string &input)
{
    return atoi(input.c_str());
}

uint32_t ConvertHexatridecimal(const string &input)
{
    return stoul(input, 0, 36);
}

double ConvertFloat(const string &input)
{
    return ToDouble(input.c_str());
}

bool ConvertBoolean(const string &input)
{
    auto test = input;
    transform(test.cbegin(), test.cend(), test.begin(), tolower);
    return
        input == "1"
        || input == "true"
        || input == "y"
        || input == "yes"
        || input == "enable"
        || input == "enabled";
}

void SplitProps(const string &source, PropList &vec)
{
    int now = 0;
    int end = 0;
    string pset;
    while (true) {
        end = source.find(',', now);
        if (end == string::npos) break;

        pset = source.substr(now, end - now);
        auto pos = pset.find(':');
        if (pos == string::npos) continue;
        vec.push_back(make_tuple(pset.substr(0, pos), pset.substr(pos + 1)));
        now = end + 1;
    }
    pset = source.substr(now);
    auto pos = pset.find(':');
    if (pos == string::npos) return;
    vec.push_back(make_tuple(pset.substr(0, pos), pset.substr(pos + 1)));
}