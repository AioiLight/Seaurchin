#pragma once

#include "SpriteManager.h"
#include "ScriptSprite.h"
#include "Easing.h"

using MoverFunction = std::function<bool(SSprite*, Mover&, double)>;

class ScriptSpriteMover final
{

private:
    std::list<std::tuple<Mover*, MoverFunction>> movers;
    static std::unordered_map<std::string, MoverFunction> actions;

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
    static bool ActionAngleTo(SSprite* target, Mover &mover, double delta);
    static bool ActionAngleBy(SSprite* target, Mover &mover, double delta);
    static bool ActionScaleTo(SSprite* target, Mover &mover, double delta);
    static bool ActionAlpha(SSprite* target, Mover &mover, double delta);
    static bool ActionDeath(SSprite* target, Mover &mover, double delta);
};