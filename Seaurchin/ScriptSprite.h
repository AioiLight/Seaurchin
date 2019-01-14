#pragma once

#include "ScriptSpriteMisc.h"
#include "MoverFunction.h"
#include "ScriptSpriteMover2.h"
#include "ScriptResource.h"

#define SU_IF_COLOR "Color"
#define SU_IF_TF2D "Transform2D"
#define SU_IF_SHAPETYPE "ShapeType"
#define SU_IF_TEXTALIGN "TextAlign"
#define SU_IF_9TYPE "NinePatchType"

#define SU_IF_SPRITE "Sprite"
#define SU_IF_SHAPE "Shape"
#define SU_IF_TXTSPRITE "TextSprite"
#define SU_IF_SYHSPRITE "SynthSprite"
#define SU_IF_CLPSPRITE "ClipSprite"
#define SU_IF_ANIMESPRITE "AnimeSprite"
#define SU_IF_CONTAINER "Container"

struct Mover;
//��ꂪImageSprite�ł������C�����Ă�񂾂�ː���
class SSprite {
private:
    virtual void DrawBy(const Transform2D &tf, const ColorTint &ct);

protected:
    int reference = 0;
    ScriptSpriteMover2 *mover = nullptr;

    void CopyParameterFrom(SSprite *original);

public:
    //�l(CopyParameterFrom�ňꊇ)
    Transform2D Transform;
    int32_t ZIndex = 0;
    ColorTint Color = Colors::white;
    bool IsDead = false;
    bool HasAlpha = true;
    //�Q��(�蓮�R�s�[)
    SImage *Image = nullptr;
    void SetImage(SImage *img);
    const SImage* GetImage() const;

    SSprite();
    virtual ~SSprite();
    void AddRef();
    void Release();

    virtual void Dismiss() { IsDead = true; }
    void Revive() { IsDead = false; }
    virtual mover_function::Action GetCustomAction(const std::string &name);
    void AddMove(const std::string &move) const;
    void AbortMove(bool terminate) const;
    void Apply(const std::string &dict);
    void Apply(const CScriptDictionary &dict);
    virtual void Tick(double delta);
    virtual void Draw();
    virtual void Draw(const Transform2D &parent, const ColorTint &color);
    virtual SSprite* Clone();

    static SSprite* Factory();
    static SSprite* Factory(SImage *img);
    static void RegisterType(asIScriptEngine *engine);
    struct Comparator {
        bool operator()(const SSprite* lhs, const SSprite* rhs) const
        {
            return lhs->ZIndex < rhs->ZIndex;
        }
    };
};

enum class SShapeType {
    Pixel,
    Box,
    BoxFill,
    Oval,
    OvalFill,
};

//�C�ӂ̑��p�`�Ȃǂ�\���ł���
class SShape : public SSprite {
private:
    void DrawBy(const Transform2D &tf, const ColorTint &ct) override;

public:
    SShapeType Type = SShapeType::BoxFill;
    double Width = 32;
    double Height = 32;

    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;

    static SShape* Factory();
    static void RegisterType(asIScriptEngine *engine);
};

enum class STextAlign {
    Top = 0,
    Center = 1,
    Bottom = 2,
    Left = 0,
    Right = 2
};

//��������X�v���C�g�Ƃ��Ĉ����܂�
class STextSprite : public SSprite {
protected:
    SRenderTarget * target = nullptr;
    SRenderTarget *scrollBuffer = nullptr;
    std::tuple<double, double, int> size;
    STextAlign horizontalAlignment = STextAlign::Left;
    STextAlign verticalAlignment = STextAlign::Top;
    bool isScrolling = false;
    int scrollWidth = 0;
    int scrollMargin = 0;
    double scrollSpeed = 0;
    double scrollPosition = 0;
    bool isRich = false;

    void Refresh();
    void DrawNormal(const Transform2D &tf, const ColorTint &ct);
    void DrawScroll(const Transform2D &tf, const ColorTint &ct);

public:
    SFont * Font = nullptr;
    std::string Text = "";
    void SetFont(SFont* font);
    void SetText(const std::string &txt);
    void SetAlignment(STextAlign hori, STextAlign vert);
    void SetRangeScroll(int width, int margin, double pps);
    void SetRich(bool enabled);

    ~STextSprite() override;
    void Tick(double delta) override;
    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    STextSprite *Clone() override;

    static STextSprite* Factory();
    static STextSprite* Factory(SFont *img, const std::string &str);
    static void RegisterType(asIScriptEngine *engine);
};

//�������͂������X�v���C�g�ł�
//���ƈ����DX���C�u�����̃��\�[�X���i�}�Ŏ擾����̂ł���܂�{�R�{�R�g��Ȃ��ł��������B
class STextInput : public SSprite {
protected:
    int inputHandle = 0;
    SFont *font = nullptr;
    int selectionStart = -1, selectionEnd = -1;
    int cursor = 0;
    std::string currentRawString = "";

public:
    STextInput();
    ~STextInput() override;
    void SetFont(SFont *font);

    void Activate() const;
    void Draw() override;
    void Tick(double delta) override;

    std::string GetUTF8String() const;

    static STextInput* Factory();
    static STextInput* Factory(SFont *img);
    static void RegisterType(asIScriptEngine *engine);
};

//�摜��C�ӂ̃X�v���C�g���獇�����ăE�F�C�ł��܂�
class SSynthSprite : public SSprite {
protected:
    SRenderTarget * target = nullptr;
    int width = 0;
    int height = 0;
    void DrawBy(const Transform2D &tf, const ColorTint &ct) override;

public:
    SSynthSprite(int w, int h);
    ~SSynthSprite() override;
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    void Clear();
    void Transfer(SSprite *sprite);
    void Transfer(SImage *image, double x, double y);
    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    SSynthSprite *Clone() override;

    static SSynthSprite *Factory(int w, int h);
    static void RegisterType(asIScriptEngine *engine);
};

//�摜��C�ӂ̃X�v���C�g���獇�����ăE�F�C�ł��܂�
class SClippingSprite : public SSynthSprite {
protected:
    double u1;
    double v1;
    double u2;
    double v2;
    SRenderTarget *actualTarget = nullptr;
    void DrawBy(const Transform2D &tf, const ColorTint &ct) override;

    static bool ActionMoveRangeTo(SSprite *thisObj, SpriteMoverArgument &args, SpriteMoverData &data, double delta);

public:
    SClippingSprite(int w, int h);

    mover_function::Action GetCustomAction(const std::string &name) override;
    void SetRange(double tx, double ty, double w, double h);
    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    SClippingSprite *Clone() override;

    static SClippingSprite *Factory(int w, int h);
    static void RegisterType(asIScriptEngine *engine);
};

class SAnimeSprite : public SSprite {
protected:
    SAnimatedImage *images;
    int loopCount, count;
    double speed;
    double time;
    void DrawBy(const Transform2D &tf, const ColorTint &ct) override;

public:
    SAnimeSprite(SAnimatedImage *img);
    ~SAnimeSprite() override;

    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    void Tick(double delta) override;
    void SetSpeed(double speed);
    void SetLoopCount(int lc);

    static SAnimeSprite* Factory(SAnimatedImage *image);
    static void RegisterType(asIScriptEngine *engine);
};

enum NinePatchType : uint32_t {
    StretchByRatio = 1,
    StretchByPixel,
    Repeat,
    RepeatAndStretch,
};

class SContainer : public SSprite {
protected:
    std::multiset<SSprite*, SSprite::Comparator> children;

public:
    SContainer();
    ~SContainer() override;

    void AddChild(SSprite *child);
    void Dismiss() override;
    void Tick(double delta) override;
    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;

    static SContainer* Factory();
    static void RegisterType(asIScriptEngine *engine);
};

template<typename T>
void RegisterSpriteBasic(asIScriptEngine *engine, const char *name)
{
    using namespace std;
    engine->RegisterObjectType(name, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(name, asBEHAVE_ADDREF, "void f()", asMETHOD(T, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(name, asBEHAVE_RELEASE, "void f()", asMETHOD(T, Release), asCALL_THISCALL);

    engine->RegisterObjectProperty(name, SU_IF_COLOR " Color", asOFFSET(T, Color));
    engine->RegisterObjectProperty(name, "bool HasAlpha", asOFFSET(T, HasAlpha));
    engine->RegisterObjectProperty(name, "int Z", asOFFSET(T, ZIndex));
    engine->RegisterObjectProperty(name, SU_IF_TF2D " Transform", asOFFSET(T, Transform));
    engine->RegisterObjectMethod(name, "void SetImage(" SU_IF_IMAGE "@)", asMETHOD(T, SetImage), asCALL_THISCALL);
    //engine->RegisterObjectMethod(name, SU_IF_IMAGE "@ get_Image()", asMETHOD(T, get_Image), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void Dismiss()", asMETHOD(T, Dismiss), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void Apply(const string &in)", asMETHODPR(T, Apply, (const std::string&), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void Apply(const dictionary@)", asMETHODPR(T, Apply, (const CScriptDictionary&), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void AddMove(const string &in)", asMETHOD(T, AddMove), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void AbortMove(bool = true)", asMETHOD(T, AbortMove), asCALL_THISCALL);
    //engine->RegisterObjectMethod(name, "void Tick(double)", asMETHOD(T, Tick), asCALL_THISCALL);
    //engine->RegisterObjectMethod(name, "void Draw()", asMETHOD(T, Draw), asCALL_THISCALL);
}



class ExecutionManager;
void RegisterScriptSprite(ExecutionManager *exm);