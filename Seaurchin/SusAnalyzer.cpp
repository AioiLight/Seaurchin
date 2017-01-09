#include "SusAnalyzer.h"

using namespace std;
namespace fsys = boost::filesystem;
namespace xp = boost::xpressive;

xp::sregex SusAnalyzer::RegexSusCommand = "#" >> (xp::s1 = +xp::alnum) >> +xp::space >> (xp::s2 = ~xp::_n);
xp::sregex SusAnalyzer::RegexSusData = "#" >> (xp::s1 = xp::repeat<3, 3>(xp::alnum)) >> (xp::s2 = xp::repeat<2, 2>(xp::alnum)) >> ":" >> *xp::space >> (xp::s3 = +(xp::alnum | xp::space));

SusAnalyzer::SusAnalyzer(uint32_t tpb)
{
    TicksPerBeat = tpb;
}

SusAnalyzer::~SusAnalyzer()
{
    Notes.clear();
}

void SusAnalyzer::Reset()
{
}

//�ꉞUTF-8�Ƃ��ď������邱�Ƃɂ��܂����ǂ����ς��Ȃ����낤�Ȃ�
//���Ɨ񋓍ς݃t�@�C���𗬂����ޑO��ŃG���[�`�F�b�N���Ȃ�
void SusAnalyzer::LoadFromFile(string fileName)
{
    ifstream file;
    string rawline;
    xp::smatch match;
    uint32_t line = 0;

    file.open(fileName, ios::in);
    while (getline(file, rawline)) {
        if (!rawline.length()) continue;
        if (rawline[0] != '#') continue;
        if (xp::regex_match(rawline, match, RegexSusCommand)) {
            ProcessCommand(match);
        }
        else if (xp::regex_match(rawline, match, RegexSusData)) {
            ProcessData(match);
        }
        else {
            ErrorCallback(0, "Error", "SUS�L���s�ł�����͂ł��܂���ł����B");
        }
    }
}

void SusAnalyzer::ProcessCommand(xp::smatch result) {


}


void SusAnalyzer::ProcessData(xp::smatch result) {


}

void SusAnalyzer::RenderScoreData()
{
}
