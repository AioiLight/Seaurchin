#pragma once

#include "Debug.h"
#include "Font.h"
#include "Misc.h"

#define SU_IF_IMAGE "Image"
#define SU_IF_FONT "Font"
#define SU_IF_RENDER "RenderTarget"
#define SU_IF_SOUND "Sound"

//interface ��������Ώ�
class ISResouceAutoRelease
{
    virtual bool Dispose() = 0;
};

//���\�[�X���N���X
class SResource
{
protected:
    int Reference = 0;
    int Handle = 0;
public:
    SResource();
    virtual ~SResource();
    void AddRef();
    void Release();

    inline int GetHandle() { return Handle; }
};

//�摜
class SImage : public SResource
{
protected:
    int Width = 0;
    int Height = 0;

    void ObtainSize();
public:
    SImage(int ih);
    ~SImage() override;

    int get_Width();
    int get_Height();

    static SImage* CreateBlankImage();
    static SImage* CreateLoadedImageFromFile(const std::string &file);
    static SImage* CreateLoadedImageFromMemory(void *buffer, size_t size);
};

//�`��^�Q
class SRenderTarget : public SImage
{
public:
    SRenderTarget(int w, int h);

    static SRenderTarget* CreateBlankTarget(int w, int h);
};

//�t�H���g
class SFont : public SResource
{
protected:
    int Size = 0;
    std::vector<GlyphInfo*> Chars;

public:
    std::vector<SImage*> Images;
    SFont();
    ~SFont() override;

    inline int get_Size() { return Size; }
    std::tuple<double, double, int> RenderRaw(SRenderTarget *rt, const std::wstring& str);

    static SFont* CreateBlankFont();
    static SFont* CreateLoadedFontFromFile(const std::string &file);
};

//����
class SSound : public SResource
{

};

//�G�t�F�N�g
class SEffect : public SResource
{
    SEffect(int handle);
    ~SEffect() override;

};

void RegisterScriptResource(asIScriptEngine *engine);