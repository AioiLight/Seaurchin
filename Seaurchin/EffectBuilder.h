#pragma once

#include "EffectData.h"

//�����ǂ��ɂ��Ȃ�˂����Ă�
using namespace boost::spirit;

class EffectBuilder final
{
private:
    std::unordered_map<std::string, EffectData*> effects;

public:
    EffectBuilder();
    ~EffectBuilder();

    void LoadFromFile(std::string fileName);
    bool ParseSource(std::string source);
};

#include "EffectGrammar.h"