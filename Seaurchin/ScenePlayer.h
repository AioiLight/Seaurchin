#pragma once

#include "Config.h"
#include "Scene.h"
#include "ScriptSprite.h"
#include "ScriptResource.h"
#include "SusAnalyzer.h"
#include "ScoreProcessor.h"
#include "SoundManager.h"
#include "MusicsManager.h"

#define SU_IF_SCENE_PLAYER "ScenePlayer"
#define SU_IF_PLAY_STATUS "PlayStatus"

#define SU_LANE_X_MIN -400.0
#define SU_LANE_X_MAX 400.0
#define SU_LANE_X_MIN_EXT -1600.0
#define SU_LANE_X_MAX_EXT 1600.0
#define SU_LANE_Z_MIN_EXT -300.0
#define SU_LANE_Z_MIN 0.0
#define SU_LANE_Z_MAX 3000.0
#define SU_LANE_Y_GROUND 0.0
#define SU_LANE_Y_AIR 240.0
#define SU_LANE_Y_AIRINDICATE 160.0
#define SU_LANE_ASPECT ((SU_LANE_Z_MAX - SU_LANE_Z_MIN) / (SU_LANE_X_MAX - SU_LANE_X_MIN))
#define SU_LANE_ASPECT_EXT ((SU_LANE_Z_MAX - SU_LANE_Z_MIN_EXT) / (SU_LANE_X_MAX - SU_LANE_X_MIN))

enum class JudgeType {
    ShortNormal = 0,
    ShortEx,
    SlideTap,
    Action,
};

enum class PlayingState {
    
    ScoreNotLoaded,     // �����n�܂��Ă��Ȃ�
    BgmNotLoaded,       // ���ʂ����ǂݍ���
    Paused,             // �|�[�Y��
    ReadyToStart,       // �ǂݍ��݂��I������̂Ŏn�߂���
    ReadyCounting,      // BGM�ǂݏI����đO�J�E���g���Ă�
    BgmPreceding,       // �O�J�E���g��������BGM�n�܂��Ă�
    OnlyScoreOngoing,   // ���ʎn�܂�������BGM�܂�
    BothOngoing,        // �����Đ����Ă�
    ScoreLasting,       // ���ʎc���Ă�
    BgmLasting,         // �Ȏc���Ă�
};

class ExecutionManager;
class ScenePlayer : public SSprite {
    friend class ScoreProcessor;
    friend class AutoPlayerProcessor;
    friend class PlayableProcessor;

protected:
    int reference = 0;
    int hGroundBuffer;
    int hBlank;
    ExecutionManager *manager;
    SoundManager *soundManager;
    std::mutex asyncMutex;
    std::unique_ptr<SusAnalyzer> analyzer;
    std::map<std::string, SResource*> resources;
    std::multiset<SSprite*, SSprite::Comparator> sprites;
    std::vector<SSprite*> spritesPending;

    SoundStream *bgmStream;
    ScoreProcessor *processor;
    bool isLoadCompleted = false;

    double cameraZ = -340, cameraY = 620, cameraTargetZ = 580; // �X�N�V������v��
    double laneBufferX = 1024;
    double laneBufferY = laneBufferX * SU_LANE_ASPECT;
    double widthPerLane = laneBufferX / 16;
    double cullingLimit = SU_LANE_ASPECT_EXT / SU_LANE_ASPECT;
    double noteImageBlockX = 64;
    double noteImageBlockY = 64;
    double scaleNoteY = 2.0;
    double actualNoteScaleX = (widthPerLane / 2) / noteImageBlockX;
    double actualNoteScaleY = actualNoteScaleX * scaleNoteY;

    SSound *soundTap, *soundExTap, *soundFlick, *soundAir, *soundAirAction, *soundHoldLoop, *soundSlideLoop;
    SImage *imageLaneGround, *imageLaneJudgeLine;
    SImage *imageTap, *imageExTap, *imageFlick, *imageHellTap;
    SImage *imageAirUp, *imageAirDown;
    SImage *imageHold, *imageHoldStrut;
    SImage *imageSlide, *imageSlideStrut;
    SImage *imageAirAction;
    SFont *fontCombo;
    SAnimatedImage *animeTap, *animeExTap, *animeSlideTap, *animeSlideLoop, *animeAirAction;
    STextSprite *textCombo;
    unsigned int slideLineColor = GetColor(0, 200, 255);
    unsigned int airActionLineColor = GetColor(0, 255, 32);
    unsigned int airActionJudgeColor = GetColor(128, 255, 160);

    //Slide�̏d�݂��኱�Ⴄ�炵�����ǂ��̂ւ񋖂��Ă�
    PlayStatus Status;

    // �Ȃ̓r���ŕω�������
    std::vector<std::shared_ptr<SusDrawableNoteData>> data;
    std::vector<std::shared_ptr<SusDrawableNoteData>> seenData, judgeData;
    std::unordered_map<std::shared_ptr<SusDrawableNoteData>, SSprite*> SlideEffects;
    // ���� ���ʒu Ex����
    std::unordered_map<std::shared_ptr<SusDrawableNoteData>, std::vector<std::tuple<double, double>>> curveData;
    double CurrentTime = 0;
    double CurrentSoundTime = 0;
    double SeenDuration = 0.8;
    double HispeedMultiplier = 6;
    double PreloadingTime = 0.5;
    double BackingTime = 0.0;
    double NextMetronomeTime = 0.0;
    double SoundBufferingLatency = 0.030;   //TODO: ���Ɏ኱���Y��
    PlayingState State = PlayingState::ScoreNotLoaded;
    bool AirActionShown = false;

    void AddSprite(SSprite *sprite);
    void SetProcessorOptions(PlayableProcessor *processor);
    void LoadResources();
    void LoadWorker();
    void RemoveSlideEffect();
    void UpdateSlideEffect();
    void CalculateNotes(double time, double duration, double preced);
    void CalculateCurves(std::shared_ptr<SusDrawableNoteData> note);
    void DrawShortNotes(std::shared_ptr<SusDrawableNoteData> note);
    void DrawAirNotes(std::shared_ptr<SusDrawableNoteData> note);
    void DrawHoldNotes(std::shared_ptr<SusDrawableNoteData> note);
    void DrawSlideNotes(std::shared_ptr<SusDrawableNoteData> note);
    void DrawAirActionNotes(std::shared_ptr<SusDrawableNoteData> note);
    void DrawTap(int lane, int length, double relpos, int handle);
    void DrawMeasureLines();
    void Prepare3DDrawCall();

    void ProcessSound();

    void SpawnJudgeEffect(std::shared_ptr<SusDrawableNoteData> target, JudgeType type);
    void SpawnSlideLoopEffect(std::shared_ptr<SusDrawableNoteData> target);
    void PlaySoundTap() { soundManager->PlayGlobal(soundTap->GetSample()); }
    void PlaySoundExTap() { soundManager->PlayGlobal(soundExTap->GetSample()); }
    void PlaySoundFlick() { soundManager->PlayGlobal(soundFlick->GetSample()); }
    void PlaySoundAir() { soundManager->PlayGlobal(soundAir->GetSample()); }
    void PlaySoundAirAction() { soundManager->PlayGlobal(soundAirAction->GetSample()); }
    void PlaySoundHold() { soundManager->PlayGlobal(soundHoldLoop->GetSample()); }
    void StopSoundHold() { soundManager->StopGlobal(soundHoldLoop->GetSample()); }
    void PlaySoundSlide() { soundManager->PlayGlobal(soundSlideLoop->GetSample()); }
    void StopSoundSlide() { soundManager->StopGlobal(soundSlideLoop->GetSample()); }

public:
    ScenePlayer(ExecutionManager *exm);
    ~ScenePlayer() override;

    void AdjustCamera(double cy, double cz, double ctz);
    void SetPlayerResource(const std::string &name, SResource *resource);
    void Tick(double delta) override;
    void Draw() override;
    void Finalize();

    void Initialize();
    void Load();
    bool IsLoadCompleted();
    void Play();
    double GetPlayingTime();
    void GetPlayStatus(PlayStatus *status);
    void MovePositionBySecond(double sec);
    void MovePositionByMeasure(int meas);
};

void RegisterPlayerScene(ExecutionManager *exm);