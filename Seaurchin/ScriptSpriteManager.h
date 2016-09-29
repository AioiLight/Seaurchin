#pragma once

#include "SpriteManager.h"
#include "ScriptSprite.h"
#include "Easing.h"

class ScriptSpriteMover final
{

private:
    std::list<std::tuple<Mover*, std::function<bool(SSprite*, Mover&, double)>>> movers;
    static std::unordered_map<std::string, std::function<bool(SSprite*, Mover&, double)>> actions;

public:
    SSprite *Target;

    ScriptSpriteMover(SSprite *target);
    ~ScriptSpriteMover();

    void AddMove(std::string move);
    //bool CheckPattern(std::string move);

    void Tick(double delta);

    //delta > 0 : ����
    //delta == 0 : Mover������
    //delta == -1 : �I��(�ʒu�m��Ȃǂɂǂ���)
    //true�Ԃ��Έ�ł����i�ł���

    static bool ActionMoveTo(SSprite* target, Mover &mover, double delta);
    static bool ActionMoveBy(SSprite* target, Mover &mover, double delta);
    static bool ActionAlpha(SSprite* target, Mover &mover, double delta);
    static bool ActionDeath(SSprite* target, Mover &mover, double delta);
};