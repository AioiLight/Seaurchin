﻿#include "SusAnalyzer.h"
#include "Misc.h"

using namespace std;
using namespace crc32_constexpr;
namespace b = boost;
namespace ba = boost::algorithm;
namespace fsys = boost::filesystem;
namespace xp = boost::xpressive;

xp::sregex SusAnalyzer::RegexSusCommand = "#" >> (xp::s1 = +xp::alnum) >> !(+xp::space >> (xp::s2 = +(~xp::_n)));
xp::sregex SusAnalyzer::RegexSusData = "#" >> (xp::s1 = xp::repeat<3, 3>(xp::alnum)) >> (xp::s2 = xp::repeat<2, 3>(xp::alnum)) >> ":" >> *xp::space >> (xp::s3 = +(~xp::_n));

static xp::sregex AllNumeric = xp::bos >> +(xp::digit) >> xp::eos;

static auto ConvertRawString = [](const string &input) -> string {
    // TIL: ASCII文字範囲ではUTF-8と本来のASCIIを間違うことはない
    if (ba::starts_with(input, "\"")) {
        ostringstream result;
        string rest = input;
        ba::trim_if(rest, ba::is_any_of("\""));
        auto it = rest.begin();
        while (it != rest.end()) {
            if (*it != '\\') {
                result << *it;
                it++;
                continue;
            }
            it++;
            if (it == rest.end()) return "";
            switch (*it) {
                case '"':
                    result << "\"";
                    break;
                case 't':
                    result << "\t";
                    break;
                case 'n':
                    result << "\n";
                    break;
                case 'u': {
                    //utf-8 4byte食う
                    char *cp = "0000";
                    for (int i = 0; i < 4; i++) {
                        cp[i] = *(++it);
                    }
                    wchar_t r = stoi(cp, 0, 16);
                    //でも突っ込むのめんどくさいので🙅で代用します
                    result << u8"🙅";
                    break;
                }
                default:
                    break;
            }
            it++;
        }
        return result.str();
    } else {
        return input;
    }
};



SusAnalyzer::SusAnalyzer(uint32_t tpb)
{
    TicksPerBeat = tpb;
    LongInjectionPerBeat = 2;
    TimelineResolver = [=](uint32_t number) { return HispeedDefinitions[number]; };
    ErrorCallbacks.push_back([this](auto type, auto message) {
        auto log = spdlog::get("main");
        log->error(message);
    });
}

SusAnalyzer::~SusAnalyzer()
{
    Reset();
}

void SusAnalyzer::Reset()
{
    Notes.clear();
    BpmDefinitions.clear();
    BeatsDefinitions.clear();
    HispeedDefinitions.clear();
    ExtraAttributes.clear();
    // TicksPerBeat = ;
    SharedMetaData.Reset();

    BpmDefinitions[0] = 120.0;
    BeatsDefinitions[0] = 4.0;

    auto defhs = make_shared<SusHispeedTimeline>([&](uint32_t m, uint32_t t) { return GetAbsoluteTime(m, t); });
    defhs->AddKeysByString("0'0:1.0:v", TimelineResolver);
    HispeedDefinitions[DefaultHispeedNumber] = defhs;
    HispeedToApply = defhs;

    auto defea = make_shared<SusNoteExtraAttribute>();
    defea->Priority = 0;
    defea->HeightScale = 1;
    ExtraAttributes[DefaultExtraAttributeNumber] = defea;
    ExtraAttributeToApply = defea;
}

void SusAnalyzer::SetMessageCallBack(function<void(string, string)> func)
{
    ErrorCallbacks.push_back(func);
}

//一応UTF-8として処理することにしますがどうせ変わらないだろうなぁ
//あと列挙済みファイルを流し込む前提でエラーチェックしない
void SusAnalyzer::LoadFromFile(const wstring &fileName, bool analyzeOnlyMetaData)
{
    auto log = spdlog::get("main");
    ifstream file;
    string rawline;
    xp::smatch match;
    uint32_t line = 0;

    Reset();
    if (!analyzeOnlyMetaData) log->info(u8"{0}の解析を開始…", ConvertUnicodeToUTF8(fileName));

    file.open(fileName, ios::in);
    char bom[3];
    file.read(bom, 3);
    if (bom[0] != (char)0xEF || bom[1] != (char)0xBB || bom[2] != (char)0xBF) file.seekg(0);
    while (getline(file, rawline)) {
        line++;
        if (!rawline.length()) continue;
        if (rawline[0] != '#') continue;
        if (xp::regex_match(rawline, match, RegexSusCommand)) {
            ProcessCommand(match, analyzeOnlyMetaData, line);
        } else if (xp::regex_match(rawline, match, RegexSusData)) {
            if (!analyzeOnlyMetaData) ProcessData(match, line);
        } else {
            MakeMessage(line, u8"SUS有効行ですが解析できませんでした。");
        }
    }
    file.close();
    if (!analyzeOnlyMetaData) log->info(u8"…終了");
    if (!analyzeOnlyMetaData) {
        stable_sort(Notes.begin(), Notes.end(), [](tuple<SusRelativeNoteTime, SusRawNoteData> a, tuple<SusRelativeNoteTime, SusRawNoteData> b) {
            return get<1>(a).Type.to_ulong() > get<1>(b).Type.to_ulong();
        });
        stable_sort(Notes.begin(), Notes.end(), [](tuple<SusRelativeNoteTime, SusRawNoteData> a, tuple<SusRelativeNoteTime, SusRawNoteData> b) {
            return get<0>(a).Tick < get<0>(b).Tick;
        });
        stable_sort(Notes.begin(), Notes.end(), [](tuple<SusRelativeNoteTime, SusRawNoteData> a, tuple<SusRelativeNoteTime, SusRawNoteData> b) {
            return get<0>(a).Measure < get<0>(b).Measure;
        });
        copy_if(Notes.begin(), Notes.end(), back_inserter(BpmChanges), [](tuple<SusRelativeNoteTime, SusRawNoteData> n) {
            return get<1>(n).Type.test((size_t)SusNoteType::Undefined);
        });

        for (auto &hs : HispeedDefinitions) hs.second->Finialize();
        if (SharedMetaData.BaseBpm == 0) SharedMetaData.BaseBpm = GetBpmAt(0, 0);
        for (const auto& bpm : BpmChanges) {
            SusRelativeNoteTime t;
            SusRawNoteData d;
            tie(t, d) = bpm;
            SharedBpmChanges.push_back(make_tuple(GetAbsoluteTime(t.Measure, t.Tick), BpmDefinitions[d.DefinitionNumber]));
        }
    }
}

void SusAnalyzer::ProcessCommand(const xp::smatch &result, bool onlyMeta, uint32_t line)
{
    auto name = result[1].str();
    transform(name.cbegin(), name.cend(), name.begin(), toupper);
    if (ba::starts_with(name, "BPM")) {
        // #BPMxx yyy.yy
        BpmDefinitions[ConvertHexatridecimal(name.substr(3))] = ConvertFloat(result[2].str());
        return;
    }
    switch (crc32_rec(0xffffffff, name.c_str())) {
        //TODO:このへんはBMSに合わせる必要あり
        case "TITLE"_crc32:
            SharedMetaData.UTitle = ConvertRawString(result[2]);
            break;
        case "SUBTITLE"_crc32:
            SharedMetaData.USubTitle = ConvertRawString(result[2]);
            break;
        case "ARTIST"_crc32:
            SharedMetaData.UArtist = ConvertRawString(result[2]);
            break;
        case "GENRE"_crc32:
            //SharedMetaData.UGenre = ConvertRawString(result[2]);
            break;
        case "DESIGNER"_crc32:
        case "SUBARTIST"_crc32:  //BMS互換
            SharedMetaData.UDesigner = ConvertRawString(result[2]);
            break;
        case "PLAYLEVEL"_crc32: {
            string lstr = result[2];
            auto pluspos = lstr.find("+");
            if (pluspos != string::npos) {
                SharedMetaData.UExtraDifficulty = u8"+";
                SharedMetaData.Level = ConvertInteger(lstr.substr(0, pluspos));
            } else {
                SharedMetaData.Level = ConvertInteger(lstr);
            }
            break;
        }
        case "DIFFICULTY"_crc32: {
            if (xp::regex_match(result[2], AllNumeric)) {
                //通常記法
                SharedMetaData.DifficultyType = ConvertInteger(result[2]);
            } else {
                //WE記法
                auto dd = ConvertRawString(result[2]);
                vector<string> params;
                ba::split(params, dd, ba::is_any_of(":"));
                if (params.size() < 2) return;
                SharedMetaData.DifficultyType = 4;
                SharedMetaData.Level = ConvertInteger(params[0]);
                SharedMetaData.UExtraDifficulty = params[1];
            }
            break;
        }
        case "SONGID"_crc32:
            SharedMetaData.USongId = ConvertRawString(result[2]);
            break;
        case "WAVE"_crc32:
            SharedMetaData.UWaveFileName = ConvertRawString(result[2]);
            break;
        case "WAVEOFFSET"_crc32:
            SharedMetaData.WaveOffset = ConvertFloat(result[2]);
            break;
        case "JACKET"_crc32:
            SharedMetaData.UJacketFileName = ConvertRawString(result[2]);
            break;
        case "BACKGROUND"_crc32:
            SharedMetaData.UBackgroundFileName = ConvertRawString(result[2]);
            break;
        case "REQUEST"_crc32:
            ProcessRequest(ConvertRawString(result[2]), line);
            break;
        case "BASEBPM"_crc32:
            SharedMetaData.BaseBpm = ConvertFloat(result[2]);
            break;

            //此処から先はデータ内で使う用
        case "HISPEED"_crc32: {
            if (onlyMeta) break;
            auto hsn = ConvertHexatridecimal(result[2]);
            if (HispeedDefinitions.find(hsn) == HispeedDefinitions.end()) {
                MakeMessage(line, u8"指定されたタイムラインが存在しません");
                break;
            }
            HispeedToApply = HispeedDefinitions[hsn];
            break;
        }
        case "NOSPEED"_crc32:
            if (!onlyMeta) HispeedToApply = HispeedDefinitions[DefaultHispeedNumber];
            break;

        case "ATTRIBUTE"_crc32: {
            if (onlyMeta) break;
            auto ean = ConvertHexatridecimal(result[2]);
            if (ExtraAttributes.find(ean) == ExtraAttributes.end()) {
                MakeMessage(line, u8"指定されたアトリビュートが存在しません");
                break;
            }
            ExtraAttributeToApply = ExtraAttributes[ean];
            break;
        }
        case "NOATTRIBUTE"_crc32:
            if (!onlyMeta) ExtraAttributeToApply = ExtraAttributes[DefaultExtraAttributeNumber];
            break;

        default:
            MakeMessage(line, u8"SUSコマンドが無効です");
            break;
    }

}

void SusAnalyzer::ProcessRequest(const string &cmd, uint32_t line)
{
    auto str = cmd;
    b::trim_if(str, ba::is_any_of(" "));
    vector<string> params;
    ba::split(params, str, ba::is_any_of(" "), b::token_compress_on);

    if (params.size() < 1) return;
    switch (crc32_rec(0xffffffff, params[0].c_str())) {
        case "mertonome"_crc32:
            if (!ConvertBoolean(params[1])) {
                SharedMetaData.ExtraFlags.set((size_t)SusMetaDataFlags::DisableMetronome);
            }
            break;
        case "ticks_per_beat"_crc32:
            TicksPerBeat = ConvertInteger(params[1]);
            break;
        case "enable_priority"_crc32:
            MakeMessage(line, u8"優先度つきノーツ描画が設定されます");
            if (ConvertBoolean(params[1])) {
                SharedMetaData.ExtraFlags.set((size_t)SusMetaDataFlags::EnableDrawPriority);
            }
            break;
    }
}

void SusAnalyzer::ProcessData(const xp::smatch &result, uint32_t line)
{
    auto meas = result[1].str();
    auto lane = result[2].str();
    auto pattern = result[3].str();
    ba::erase_all(pattern, " ");

    /*
     判定順について
     0. #...** (BPMなど)
     1. #---0* (特殊データ、定義分割不可)
     2. #---1* (Short)
     3. #---5* (Air)
     4. #---[234]*. (Long)
    */

    auto noteCount = pattern.length() / 2;
    auto step = (uint32_t)(TicksPerBeat * GetBeatsAt(ConvertInteger(meas))) / (!noteCount ? 1 : noteCount);

    if (!xp::regex_match(meas, AllNumeric)) {
        // コマンドデータ
        transform(meas.cbegin(), meas.cend(), meas.begin(), toupper);
        if (meas == "BPM") {
            auto number = ConvertHexatridecimal(lane);
            BpmDefinitions[number] = ConvertFloat(pattern);
        } else if (meas == "TIL") {
            auto number = ConvertHexatridecimal(lane);
            auto it = HispeedDefinitions.find(number);
            if (it == HispeedDefinitions.end()) {
                auto hs = make_shared<SusHispeedTimeline>([&](uint32_t m, uint32_t t) { return GetAbsoluteTime(m, t); });
                hs->AddKeysByString(ConvertRawString(pattern), TimelineResolver);
                HispeedDefinitions[number] = hs;
            } else {
                it->second->AddKeysByString(ConvertRawString(pattern), TimelineResolver);
            }
        } else if (meas == "ATR") {
            auto number = ConvertHexatridecimal(lane);
            auto it = ExtraAttributes.find(number);
            if (it == ExtraAttributes.end()) {
                auto ea = make_shared<SusNoteExtraAttribute>();
                ea->Apply(ConvertRawString(pattern));
                ExtraAttributes[number] = ea;
            } else {
                it->second->Apply(ConvertRawString(pattern));
            }
        } else {
            MakeMessage(line, u8"不正なデータコマンドです");
        }
    } else if (lane[0] == '0') {
        switch (lane[1]) {
            case '2':
                // 小節長
                BeatsDefinitions[ConvertInteger(meas)] = ConvertFloat(pattern);
                break;
            case '8': {
                // BPM
                for (auto i = 0; i < noteCount; i++) {
                    auto note = pattern.substr(i * 2, 2);
                    SusRawNoteData noteData;
                    SusRelativeNoteTime time = { ConvertInteger(meas), step * i };
                    noteData.Type.set((size_t)SusNoteType::Undefined);
                    noteData.DefinitionNumber = ConvertHexatridecimal(note);
                    if (noteData.DefinitionNumber) Notes.push_back(make_tuple(time, noteData));
                }
                break;
            }
            default:
                MakeMessage(line, u8"不正なデータコマンドです");
                break;
        }
    } else if (lane[0] == '1') {
        // ショートノーツ
        for (auto i = 0; i < noteCount; i++) {
            auto note = pattern.substr(i * 2, 2);
            SusRawNoteData noteData;
            SusRelativeNoteTime time = { ConvertInteger(meas), step * i };
            noteData.NotePosition.StartLane = ConvertHexatridecimal(lane.substr(1, 1));
            noteData.NotePosition.Length = ConvertHexatridecimal(note.substr(1, 1));
            noteData.Timeline = HispeedToApply;
            noteData.ExtraAttribute = ExtraAttributeToApply;

            switch (note[0]) {
                case '1':
                    noteData.Type.set((size_t)SusNoteType::Tap);
                    break;
                case '2':
                    noteData.Type.set((size_t)SusNoteType::ExTap);
                    break;
                case '3':
                    noteData.Type.set((size_t)SusNoteType::Flick);
                    break;
                case '4':
                    // 本来はHell
                    noteData.Type.set((size_t)SusNoteType::HellTap);
                    break;
                default:
                    if (note[1] == '0') continue;
                    MakeMessage(line, u8"ショートレーンの指定が不正です。");
                    continue;
            }
            Notes.push_back(make_tuple(time, noteData));
        }
    } else if (lane[0] == '5') {
        // Airノーツ
        for (auto i = 0; i < noteCount; i++) {
            auto note = pattern.substr(i * 2, 2);
            SusRawNoteData noteData;
            SusRelativeNoteTime time = { ConvertInteger(meas), step * i };
            noteData.NotePosition.StartLane = ConvertHexatridecimal(lane.substr(1, 1));
            noteData.NotePosition.Length = ConvertHexatridecimal(note.substr(1, 1));
            noteData.Timeline = HispeedToApply;
            noteData.ExtraAttribute = ExtraAttributeToApply;

            switch (note[0]) {
                case '1':
                    noteData.Type.set((size_t)SusNoteType::Air);
                    noteData.Type.set((size_t)SusNoteType::Up);
                    break;
                case '2':
                    noteData.Type.set((size_t)SusNoteType::Air);
                    noteData.Type.set((size_t)SusNoteType::Down);
                    break;
                case '3':
                    noteData.Type.set((size_t)SusNoteType::Air);
                    noteData.Type.set((size_t)SusNoteType::Up);
                    noteData.Type.set((size_t)SusNoteType::Left);
                    break;
                case '4':
                    noteData.Type.set((size_t)SusNoteType::Air);
                    noteData.Type.set((size_t)SusNoteType::Up);
                    noteData.Type.set((size_t)SusNoteType::Right);
                    break;
                case '5':
                    noteData.Type.set((size_t)SusNoteType::Air);
                    noteData.Type.set((size_t)SusNoteType::Down);
                    noteData.Type.set((size_t)SusNoteType::Right);
                    break;
                case '6':
                    noteData.Type.set((size_t)SusNoteType::Air);
                    noteData.Type.set((size_t)SusNoteType::Down);
                    noteData.Type.set((size_t)SusNoteType::Left);
                    break;
                default:
                    if (note[1] == '0') continue;
                    MakeMessage(line, u8"Airレーンの指定が不正です。");
                    continue;
            }
            Notes.push_back(make_tuple(time, noteData));
        }
    } else if (lane.length() == 3) {
        // ロングタイプ
        for (auto i = 0; i < noteCount; i++) {
            auto note = pattern.substr(i * 2, 2);
            SusRawNoteData noteData;
            SusRelativeNoteTime time = { ConvertInteger(meas), step * i };
            noteData.NotePosition.StartLane = ConvertHexatridecimal(lane.substr(1, 1));
            noteData.NotePosition.Length = ConvertHexatridecimal(note.substr(1, 1));
            noteData.Extra = ConvertHexatridecimal(lane.substr(2, 1));
            noteData.Timeline = HispeedToApply;
            noteData.ExtraAttribute = ExtraAttributeToApply;

            switch (lane[0]) {
                case '2':
                    noteData.Type.set((size_t)SusNoteType::Hold);
                    break;
                case '3':
                    noteData.Type.set((size_t)SusNoteType::Slide);
                    break;
                case '4':
                    noteData.Type.set((size_t)SusNoteType::AirAction);
                    break;
                default:
                    MakeMessage(line, u8"ロングレーンの指定が不正です。");
                    continue;
            }
            switch (note[0]) {
                case '1':
                    noteData.Type.set((size_t)SusNoteType::Start);
                    break;
                case '2':
                    noteData.Type.set((size_t)SusNoteType::End);
                    break;
                case '3':
                    noteData.Type.set((size_t)SusNoteType::Step);
                    break;
                case '4':
                    noteData.Type.set((size_t)SusNoteType::Control);
                    break;
                case '5':
                    noteData.Type.set((size_t)SusNoteType::Invisible);
                    break;
                default:
                    if (note[1] == '0') continue;
                    MakeMessage(line, u8"ノーツ種類の指定が不正です。");
                    continue;
            }
            Notes.push_back(make_tuple(time, noteData));
        }
    } else {
        // 不正
        MakeMessage(line, u8"不正なデータ定義です。");
    }
}

void SusAnalyzer::MakeMessage(uint32_t line, const string &message)
{
    ostringstream ss;
    ss << line << u8"行目: " << message;
    for (const auto &cb : ErrorCallbacks) cb("Error", ss.str());
}

void SusAnalyzer::MakeMessage(uint32_t meas, uint32_t tick, uint32_t lane, const std::string &message)
{
    ostringstream ss;
    ss << meas << u8"'" << tick << u8"@" << lane << u8": " << message;
    for (const auto &cb : ErrorCallbacks) cb("Error", ss.str());
}

float SusAnalyzer::GetBeatsAt(uint32_t measure)
{
    float result = DefaultBeats;
    uint32_t last = 0;
    for (auto &t : BeatsDefinitions) {
        if (t.first >= last && t.first <= measure) {
            result = t.second;
            last = t.first;
        }
    }
    return result;
}

double SusAnalyzer::GetBpmAt(uint32_t measure, uint32_t tick)
{
    double result = DefaultBpm;
    for (auto &t : BpmChanges) {
        if (get<0>(t).Measure != measure) continue;
        if (get<0>(t).Tick < tick) continue;
        result = BpmDefinitions[get<1>(t).DefinitionNumber];
    }
    return result;
}

double SusAnalyzer::GetAbsoluteTime(uint32_t meas, uint32_t tick)
{
    double time = 0.0;
    double lastBpm = DefaultBpm;
    //超過したtick指定にも対応したほうが使いやすいよね
    while (tick >= GetBeatsAt(meas) * TicksPerBeat) tick -= GetBeatsAt(meas++) * TicksPerBeat;

    for (int i = 0; i < meas + 1; i++) {
        auto beats = GetBeatsAt(i);
        auto lastChangeTick = 0u;
        for (auto& bc : BpmChanges) {
            if (get<0>(bc).Measure != i) continue;
            auto timing = get<0>(bc);
            if (i == meas && timing.Tick >= tick) break;
            double dur = (60.0 / lastBpm) * ((double)(timing.Tick - lastChangeTick) / TicksPerBeat);
            time += dur;
            lastChangeTick = timing.Tick;
            lastBpm = BpmDefinitions[get<1>(bc).DefinitionNumber];
        }
        if (i == meas) {
            time += (60.0 / lastBpm) * ((double)(tick - lastChangeTick) / TicksPerBeat);
        } else {
            time += (60.0 / lastBpm) * ((double)(TicksPerBeat * beats - lastChangeTick) / TicksPerBeat);
        }
    }

    return time;
};

tuple<uint32_t, uint32_t> SusAnalyzer::GetRelativeTime(double time)
{
    double restTime = time;
    uint32_t meas = 0, tick = 0;
    double secPerBeat = (60.0 / 120.0);

    while (true) {
        auto beats = GetBeatsAt(meas);
        auto lastChangeTick = 0u;

        for (auto& bc : BpmChanges) {
            auto timing = get<0>(bc);
            if (timing.Measure != meas) continue;
            double dur = secPerBeat * ((double)(timing.Tick - lastChangeTick) / TicksPerBeat);
            if (dur >= restTime) return make_tuple(meas, lastChangeTick + restTime / secPerBeat * TicksPerBeat);
            restTime -= dur;
            lastChangeTick = timing.Tick;
            secPerBeat = 60.0 / BpmDefinitions[get<1>(bc).DefinitionNumber];
        }
        double restTicks = TicksPerBeat * beats - lastChangeTick;
        double restDuration = restTicks / TicksPerBeat * secPerBeat;
        if (restDuration >= restTime) return make_tuple(meas, lastChangeTick + restTime / secPerBeat * TicksPerBeat);
        restTime -= restDuration;
        meas++;
    }
}

uint32_t SusAnalyzer::GetRelativeTicks(uint32_t measure, uint32_t tick)
{
    uint32_t result = 0;
    for (int i = 0; i < measure; i++) result += GetBeatsAt(i) * TicksPerBeat;
    return result + tick;
}

void SusAnalyzer::RenderScoreData(vector<shared_ptr<SusDrawableNoteData>> &data)
{
    // 不正チェックリスト
    // ショート: はみ出しは全部アウト
    // ホールド: ケツ無しアウト(ケツ連は無視)、Step/Control問答無用アウト、ケツ違いアウト
    // スライド、AA: ケツ無しアウト(ケツ連は無視)
    data.clear();
    for (auto& note : Notes) {
        auto time = get<0>(note);
        auto info = get<1>(note);
        if (info.Type[(size_t)SusNoteType::Step]) continue;
        if (info.Type[(size_t)SusNoteType::Control]) continue;
        if (info.Type[(size_t)SusNoteType::Invisible]) continue;
        if (info.Type[(size_t)SusNoteType::End]) continue;
        if (info.Type[(size_t)SusNoteType::Undefined]) continue;

        auto bits = info.Type.to_ulong();
        auto noteData = make_shared<SusDrawableNoteData>();
        if (bits & SU_NOTE_LONG_MASK) {
            noteData->Type = info.Type;
            noteData->StartTime = GetAbsoluteTime(time.Measure, time.Tick);
            noteData->StartLane = info.NotePosition.StartLane;
            noteData->Length = info.NotePosition.Length;
            noteData->Timeline = info.Timeline;
            noteData->ExtraAttribute = info.ExtraAttribute;
            noteData->StartTimeEx = get<1>(noteData->Timeline->GetRawDrawStateAt(noteData->StartTime));

            SusNoteType ltype;
            switch ((bits >> 6) & 7) {
                case 1:
                    ltype = SusNoteType::Hold;
                    break;
                case 2:
                    ltype = SusNoteType::Slide;
                    break;
                case 4:
                    ltype = SusNoteType::AirAction;
                    break;
            }

            bool completed = false;
            auto lastStep = note;
            for (auto it : Notes) {
                auto curPos = get<0>(it);
                auto curNo = get<1>(it);
                if (!curNo.Type.test((size_t)ltype) || curNo.Extra != info.Extra) continue;
                if (curPos.Measure < time.Measure) continue;
                if (curPos.Measure == time.Measure && curPos.Tick < time.Tick) continue;
                switch (ltype) {
                    case SusNoteType::Hold: {
                        if (curNo.Type.test((size_t)SusNoteType::Control) || curNo.Type.test((size_t)SusNoteType::Invisible))
                            MakeMessage(curPos.Measure, curPos.Tick, curNo.NotePosition.StartLane, u8"HoldでControl/Invisibleは指定できません。");
                        if (curNo.NotePosition.StartLane != info.NotePosition.StartLane || curNo.NotePosition.Length != info.NotePosition.Length)
                            MakeMessage(curPos.Measure, curPos.Tick, curNo.NotePosition.StartLane, u8"Holdの長さ/位置が始点と一致していません。");
                    }
                                            /* ホールドだけ追加チェックしてフォールスルー */
                    case SusNoteType::Slide:
                    case SusNoteType::AirAction: {
                        if (curNo.Type.test((size_t)SusNoteType::Start)) break;

                        auto nextNote = make_shared<SusDrawableNoteData>();
                        nextNote->StartTime = GetAbsoluteTime(curPos.Measure, curPos.Tick);
                        nextNote->StartLane = curNo.NotePosition.StartLane;
                        nextNote->Length = curNo.NotePosition.Length;
                        nextNote->Type = curNo.Type;
                        nextNote->Timeline = curNo.Timeline;
                        nextNote->ExtraAttribute = curNo.ExtraAttribute;
                        nextNote->StartTimeEx = get<1>(nextNote->Timeline->GetRawDrawStateAt(nextNote->StartTime));

                        if (curNo.Type.test((size_t)SusNoteType::Step) || curNo.Type.test((size_t)SusNoteType::End)) {
                            auto lsrt = get<0>(lastStep);
                            auto injc = (double)(GetRelativeTicks(curPos.Measure, curPos.Tick) - GetRelativeTicks(lsrt.Measure, lsrt.Tick)) / TicksPerBeat * LongInjectionPerBeat;
                            for (int i = 1; i < injc; i++) {
                                double insertAt = lsrt.Tick + (TicksPerBeat / LongInjectionPerBeat * i);
                                auto injection = make_shared<SusDrawableNoteData>();
                                injection->Type.set((size_t)SusNoteType::Injection);
                                injection->StartTime = GetAbsoluteTime(lsrt.Measure, insertAt);
                                noteData->ExtraData.push_back(injection);
                            }
                        }

                        if (nextNote->Type.test((size_t)SusNoteType::Step)) lastStep = it;

                        if (curNo.Type.test((size_t)SusNoteType::End)) {
                            noteData->Duration = nextNote->StartTime - noteData->StartTime;
                            completed = true;
                        }

                        noteData->ExtraData.push_back(nextNote);
                        break;
                    }
                }
                if (completed) break;
            }
            if (!completed) {
                MakeMessage(time.Measure, time.Tick, info.NotePosition.StartLane, u8"ロングノーツに終点がありません。");
            } else {
                data.push_back(noteData);
            }
        } else if (bits & SU_NOTE_SHORT_MASK) {
            // ショート
            if (info.NotePosition.StartLane + info.NotePosition.Length > 16) {
                MakeMessage(time.Measure, time.Tick, info.NotePosition.StartLane, u8"ショートノーツがはみ出しています。");
            }
            noteData->Type = info.Type;
            noteData->StartTime = GetAbsoluteTime(time.Measure, time.Tick);
            noteData->Duration = 0;
            noteData->StartLane = info.NotePosition.StartLane;
            noteData->Length = info.NotePosition.Length;
            noteData->Timeline = info.Timeline;
            noteData->ExtraAttribute = info.ExtraAttribute;
            noteData->StartTimeEx = get<1>(noteData->Timeline->GetRawDrawStateAt(noteData->StartTime));
            data.push_back(noteData);

        } else {
            MakeMessage(time.Measure, time.Tick, info.NotePosition.StartLane, u8"致命的なノーツエラー(不正な内部表現です)。");
        }
        auto test = GetRelativeTime(GetAbsoluteTime(2, 200));
    }
}

// SusHispeedTimeline ------------------------------------------------------------------

const double SusHispeedData::KeepSpeed = numeric_limits<double>::quiet_NaN();

SusHispeedTimeline::SusHispeedTimeline(std::function<double(uint32_t, uint32_t)> func) : RelToAbs(func)
{
    keys.push_back(make_pair(SusRelativeNoteTime { 0, 0 }, SusHispeedData { SusHispeedData::Visibility::Visible, 1.0 }));
}

void SusHispeedTimeline::AddKeysByString(const string & def, function<shared_ptr<SusHispeedTimeline>(uint32_t)> resolver)
{
    //int'int:double:v/i
    string str = def;
    vector<string> ks;

    ba::erase_all(str, " ");
    ba::split(ks, str, b::is_any_of(","));
    for (const auto &k : ks) {
        vector<string> params;
        ba::split(params, k, b::is_any_of(":"));
        if (params.size() < 2) return;
        if (params[0] == "inherit") {
            //データ流用
            auto from = ConvertHexatridecimal(params[1]);
            auto parent = resolver(from);
            if (!parent) continue;
            for (auto &parentKey : parent->keys) keys.push_back(parentKey);
            continue;
        }

        vector<string> timing;
        ba::split(timing, params[0], b::is_any_of("'"));
        SusRelativeNoteTime time = { ConvertInteger(timing[0]), ConvertInteger(timing[1]) };
        SusHispeedData data = { SusHispeedData::Visibility::Keep, SusHispeedData::KeepSpeed };
        for (int i = 1; i < params.size(); i++) {
            if (params[i] == "v" || params[i] == "visible") {
                data.VisibilityState = SusHispeedData::Visibility::Visible;
            } else if (params[i] == "i" || params[i] == "invisible") {
                data.VisibilityState = SusHispeedData::Visibility::Invisible;
            } else {
                data.Speed = ConvertFloat(params[i]);
            }
        }
        bool found = false;
        for (auto &p : keys) {
            if (p.first == time) {
                p.second = data;
                found = true;
                break;
            }
        }
        if (!found) keys.push_back(make_pair(time, data));
    }
}

void SusHispeedTimeline::AddKeyByData(uint32_t meas, uint32_t tick, double hs)
{
    SusRelativeNoteTime time = { meas, tick };
    for (auto &p : keys) {
        if (p.first != time) continue;
        p.second.Speed = hs;
        return;
    }
    SusHispeedData data = { SusHispeedData::Visibility::Keep, hs };
    keys.push_back(make_pair(time, data));
}

void SusHispeedTimeline::AddKeyByData(uint32_t meas, uint32_t tick, bool vis)
{
    SusRelativeNoteTime time = { meas, tick };
    auto vv = vis ? SusHispeedData::Visibility::Visible : SusHispeedData::Visibility::Invisible;
    for (auto &p : keys) {
        if (p.first != time) continue;
        p.second.VisibilityState = vv;
        return;
    }
    SusHispeedData data = { vv, SusHispeedData::KeepSpeed };
    keys.push_back(make_pair(time, data));
}

void SusHispeedTimeline::Finialize()
{
    stable_sort(keys.begin(), keys.end(), [](const pair<SusRelativeNoteTime, SusHispeedData> &a, const pair<SusRelativeNoteTime, SusHispeedData> &b) {
        return a.first.Tick < b.first.Tick;
    });
    stable_sort(keys.begin(), keys.end(), [](const pair<SusRelativeNoteTime, SusHispeedData> &a, const pair<SusRelativeNoteTime, SusHispeedData> &b) {
        return a.first.Measure < b.first.Measure;
    });
    double hs = 1.0;
    bool vis = true;
    for (auto &key : keys) {
        if (!isnan(key.second.Speed)) {
            hs = key.second.Speed;
        } else {
            key.second.Speed = hs;
        }
        if (key.second.VisibilityState != SusHispeedData::Visibility::Keep) {
            vis = key.second.VisibilityState == SusHispeedData::Visibility::Visible;
        } else {
            key.second.VisibilityState = vis ? SusHispeedData::Visibility::Visible : SusHispeedData::Visibility::Invisible;
        }
    }

    auto it = keys.begin();
    double sum = 0;
    double lastAt = 0;
    bool lastVisible = true;
    double lastSpeed = 1.0;
    for (auto &rd : keys) {
        double t = RelToAbs(rd.first.Measure, rd.first.Tick);
        sum += (t - lastAt) * lastSpeed;
        Data.push_back(make_tuple(t, sum, rd.second));
        lastAt = t;
        lastSpeed = rd.second.Speed;
    }
    keys.clear();
}

tuple<bool, double> SusHispeedTimeline::GetRawDrawStateAt(double time)
{
    auto lastData = Data[0];
    int check = 0;
    for (auto &d : Data) {
        if (!check++) continue;
        double keyTime = get<0>(d);
        if (keyTime >= time) break;
        lastData = d;
    }
    double lastDifference = time - get<0>(lastData);
    return make_tuple(get<2>(lastData).VisibilityState == SusHispeedData::Visibility::Visible, get<1>(lastData) + lastDifference * get<2>(lastData).Speed);
}

double SusHispeedTimeline::GetSpeedAt(double time)
{
    auto lastData = Data[0];
    int check = 0;
    for (auto &d : Data) {
        if (!check++) continue;
        double keyTime = get<0>(d);
        if (keyTime >= time) break;
        lastData = d;
    }
    return get<2>(lastData).Speed;
}

tuple<bool, double> SusDrawableNoteData::GetStateAt(double time)
{
    auto result = Timeline->GetRawDrawStateAt(time);
    ModifiedPosition = StartTimeEx - get<1>(result);
    for (auto &ex : ExtraData) {
        if (!ex->Timeline) {
            ex->ModifiedPosition = numeric_limits<double>::quiet_NaN();
            continue;
        }
        auto eres = ex->Timeline->GetRawDrawStateAt(time);
        ex->ModifiedPosition = ex->StartTimeEx - get<1>(eres);
    }
    return make_tuple(get<0>(result), StartTimeEx - get<1>(result));
}

void SusNoteExtraAttribute::Apply(const string &props)
{
    using namespace boost::algorithm;
    string list = props;
    list.erase(remove(list.begin(), list.end(), ' '), list.end());
    vector<string> params;
    split(params, list, is_any_of(","));

    vector<string> pr;
    for (auto& p : params) {
        pr.clear();
        split(pr, p, is_any_of(":"));
        if (pr.size() != 2) continue;
        switch (crc32_rec(0xffffffff, pr[0].c_str())) {
            case "priority"_crc32:
            case "pr"_crc32:
                Priority = (uint32_t)atoi(pr[1].c_str());
                break;
            case "height"_crc32:
            case "h"_crc32:
                HeightScale = (double)atof(pr[1].c_str());
                break;
        }
    }
}
