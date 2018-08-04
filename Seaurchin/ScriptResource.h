#pragma once

#include "Font.h"
#include "SoundManager.h"
#include "Setting.h"
#include "ScriptSpriteMisc.h"

#define SU_IF_IMAGE "Image"
#define SU_IF_FONT "Font"
#define SU_IF_RENDER "RenderTarget"
#define SU_IF_SOUNDMIXER "SoundMixer"
#define SU_IF_SOUND "Sound"
#define SU_IF_ANIMEIMAGE "AnimatedImage"
#define SU_IF_SETTING_ITEM "SettingItem"

//interface ��������Ώ�
class SResouceAutoRelease {
public:
    virtual ~SResouceAutoRelease() = default;
private:
    virtual bool Dispose() = 0;
};

//���\�[�X���N���X
class SResource {
protected:
    int reference = 0;
    int handle = 0;
public:
    SResource();
    virtual ~SResource();
    void AddRef();
    void Release();

    int GetHandle() const { return handle; }
};

//�摜
class SImage : public SResource {
protected:
    int width = 0;
    int height = 0;

    void ObtainSize();
public:
    explicit SImage(int ih);
    ~SImage() override;

    int GetWidth();
    int GetHeight();

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
    int leftSideWidth = 8;
    int topSideHeight = 8;
    int bodyWidth = 32;
    int bodyHeight = 32;

public:
    explicit SNinePatchImage(int ih);
    ~SNinePatchImage() override;
    void SetArea(int leftw, int toph, int bodyw, int bodyh);
    std::tuple<int, int, int, int> GetArea() { return std::make_tuple(leftSideWidth, topSideHeight, bodyWidth, bodyHeight); }
};

//�A�j���[�V�����p
class SAnimatedImage : public SImage {
protected:
    int cellWidth = 0;
    int cellHeight = 0;
    int frameCount = 0;
    double secondsPerFrame = 0.1;
    std::vector<int> images;

public:
    SAnimatedImage(int w, int h, int count, double time);
    ~SAnimatedImage() override;

    double GetCellTime() const { return secondsPerFrame; }
    int GetFrameCount() const { return frameCount; }
    int GetImageHandleAt(const double time) { return images[int(time / secondsPerFrame) % frameCount]; }

    static SAnimatedImage *CreateLoadedImageFromFile(const std::string &file, int xc, int yc, int w, int h, int count, double time);
};

//�t�H���g
class SFont : public SResource {
protected:
    int size = 0;
    std::unordered_map<uint32_t, Sif2Glyph*> glyphs;

public:
    std::vector<SImage*> Images;
    SFont();
    ~SFont() override;

    int GetSize() const { return size; }
    std::tuple<double, double, int> RenderRaw(SRenderTarget *rt, const std::string& utf8Str);
    std::tuple<double, double, int> RenderRich(SRenderTarget *rt, const std::string& utf8Str, const ColorTint &defcol);

    static SFont* CreateBlankFont();
    static SFont* CreateLoadedFontFromFile(const std::string &file);
};

class SSound : public SResource {
    friend class SSoundMixer;

protected:
    SoundSample *sample;

public:
    SSound(SoundSample *smp);
    ~SSound() override;

    SoundSample *GetSample() const { return sample; }
    void SetLoop(bool looping) const;
    void SetVolume(float vol) const;

    static SSound* CreateSound(SoundManager *smanager);
    static SSound* CreateSoundFromFile(SoundManager *smanager, const std::string &file, int simul);
};

class SSoundMixer : public SResource {
protected:
    SoundMixerStream *mixer;

public:
    SSoundMixer(SoundMixerStream *mixer);
    ~SSoundMixer() override;

    void Update() const;
    void Play(SSound *sound) const;
    void Stop(SSound *sound) const;

    static SSoundMixer *CreateMixer(SoundManager *manager);
};

class SSettingItem : public SResource {
protected:
    std::shared_ptr<setting2::SettingItem> setting;

public:
    SSettingItem(std::shared_ptr<setting2::SettingItem> s);
    ~SSettingItem() override;

    void Save() const;
    void MoveNext() const;
    void MovePrevious() const;
    std::string GetItemText() const;
    std::string GetDescription() const;
};

class ExecutionManager;
void RegisterScriptResource(ExecutionManager *exm);