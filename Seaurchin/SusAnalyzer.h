#pragma once

enum SusNoteType : uint8_t {
    Undefined = 0,
    Tap,
    ExTap,
    HoldStart,
    HoldEnd,
    Air,
    Action,
    SlideStart,
    SlideChange,
    SlideControl,
    SlideEnd,
    //�����܂ł�0x0A

    //�⏕���� ���Air������ތn��
    Up = 0b00010000,
    Down = 0b00100000,
    Left = 0b01000000,
    Right = 0b10000000,
};

struct SusNoteTime {
    uint32_t Measure;
    uint32_t Tick;
};

struct SusNoteData {
    SusNoteType Type;
    uint8_t StartLane;
    uint8_t Length;
    uint8_t Extra;
};

struct SusMetaData {
    std::string UTitle;
    std::string USubTitle;
    std::string UArtist;
    std::string UDesigner;
    std::string USongId;
    uint32_t Level;
    uint32_t DifficultyType;
};

//BMS�h���t�H�[�}�b�g����SUS(SeaUrchinScore)�̉��
class SusAnalyzer final {
private:
    static boost::xpressive::sregex RegexSusCommand;
    static boost::xpressive::sregex RegexSusData;

    uint32_t TicksPerBeat;
    std::function<void(uint32_t, std::string, std::string)> ErrorCallback = nullptr;
    std::vector<std::tuple<SusNoteTime, SusNoteData>> Notes;
    std::unordered_map<uint32_t, double> BpmDefinitions;

    void ProcessCommand(boost::xpressive::smatch result);
    void ProcessData(boost::xpressive::smatch result);

public:
    SusMetaData SharedMetaData;

    SusAnalyzer(uint32_t tpb);
    ~SusAnalyzer();

    void Reset();
    void LoadFromFile(std::string fileName);
    void RenderScoreData();
};