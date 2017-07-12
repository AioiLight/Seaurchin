#pragma once

#include "Debug.h"
#include "Font.h"
#include "EffectData.h"
#include "SoundManager.h"
#include "Misc.h"
#include "Setting.h"

#define SU_IF_IMAGE "Image"
#define SU_IF_FONT "Font"
#define SU_IF_RENDER "RenderTarget"
#define SU_IF_SOUNDMIXER "SoundMixer"
#define SU_IF_SOUND "Sound"
#define SU_IF_EFXDATA "EffectData"
#define SU_IF_9IMAGE "NinePatchImage"
#define SU_IF_ANIMEIMAGE "AnimatedImage"
#define SU_IF_SETTING_ITEM "SettingItem"

//interface ��������Ώ�
class ISResouceAutoRelease {
    virtual bool Dispose() = 0;
};

//���\�[�X���N���X
class SResource {
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
class SImage : public SResource {
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
    static SImage* CreateLoadedImageFromFile(const std::string &file, bool async);
    static SImage* CreateLoadedImageFromMemory(void *buffer, size_t size);
};

//�`��^�Q
class SRenderTarget : public SImage {
public:
    SRenderTarget(int w, int h);

    static SRenderTarget* CreateBlankTarget(int w, int h);
};

//9patch�`��p
class SNinePatchImage : public SImage {
protected:
    int LeftSideWidth = 8;
    int TopSideHeight = 8;
    int BodyWidth = 32;
    int BodyHeight = 32;

public:
    SNinePatchImage(int ih);
    ~SNinePatchImage() override;
    void SetArea(int leftw, int toph, int bodyw, int bodyh);
    std::tuple<int, int, int, int> GetArea() { return std::make_tuple(LeftSideWidth, TopSideHeight, BodyWidth, BodyHeight); }
};

//�A�j���[�V�����p
class SAnimatedImage : public SImage {
protected:
    int CellWidth = 0;
    int CellHeight = 0;
    int FrameCount = 0;
    double SecondsPerFrame = 0.1;
    std::vector<int> Images;

public:
    SAnimatedImage(int w, int h, int count, double time);
    ~SAnimatedImage() override;

    double get_CellTime() { return SecondsPerFrame; }
    int get_FrameCount() { return FrameCount; }
    int GetImageHandleAt(double time) { return Images[(int)(time / SecondsPerFrame) % FrameCount]; }

    static SAnimatedImage *CreateLoadedImageFromFile(const std::string &file, int xc, int yc, int w, int h, int count, double time);
};

//�t�H���g
class SFont : public SResource {
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

//�G�t�F�N�g
class SEffect : public SResource {
protected:

public:
    EffectData *data;

    SEffect(EffectData *rawdata);
    ~SEffect() override;
};

class SSound : public SResource {
    friend class SSoundMixer;

protected:
    SoundSample *sample;

public:
    SSound(SoundSample *smp);
    ~SSound() override;

    inline SoundSample *GetSample() { return sample; }
    void SetLoop(bool looping);

    static SSound* CreateSound(SoundManager *smanager);
    static SSound* CreateSoundFromFile(SoundManager *smanager, const std::string &file, int simul);
};

class SSoundMixer : public SResource {
protected:
    SoundMixerStream *mixer;

public:
    SSoundMixer(SoundMixerStream *mixer);
    ~SSoundMixer() override;

    void Update();
    void Play(SSound *sound);
    void Stop(SSound *sound);

    static SSoundMixer *CreateMixer(SoundManager *manager);
};

class SSettingItem : public SResource {
protected:
    std::shared_ptr<SettingItem> setting;

public:
    SSettingItem(std::shared_ptr<SettingItem> s);
    ~SSettingItem() override;

    void Save();
    void MoveNext();
    void MovePrevious();
    std::string GetItemText();
    std::string GetDescription();
};

class ExecutionManager;
void RegisterScriptResource(ExecutionManager *exm);