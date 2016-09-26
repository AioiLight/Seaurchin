#pragma once

#include "SpriteManager.h"
#include "ScriptSprite.h"
#include "Easing.h"

class ScriptSpriteManager final
{

private:
    std::list<std::tuple<SSprite*, Mover*, std::function<bool(SSprite*, Mover&, double)>>> movers;
    static std::unordered_map<std::string, std::function<bool(SSprite*, Mover&, double)>> actions;

public:
    ScriptSpriteManager();
    void AddMove(SSprite* sprite, std::string move);
    //bool CheckPattern(std::string move);

    void Tick(double delta);

    //delta > 0 : ����
    //delta == 0 : Mover������
    //delta == -1 : �I��(�ʒu�m��Ȃǂɂǂ���)
    //true�Ԃ��Έ�ł����i�ł���

    static bool ActionMoveTo(SSprite* target, Mover &mover, double delta);
    static bool ActionMoveBy(SSprite* target, Mover &mover, double delta);
    static bool ActionAlpha(SSprite* target, Mover &mover, double delta);
};