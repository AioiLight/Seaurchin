#include "ScoreProcessor.h"
#include "ExecutionManager.h"
#include "ScenePlayer.h"

using namespace std;

// PlayStatus -------------------------------------------------

void PlayStatus::GetGaugeValue(int &fulfilled, double &rest)
{
    fulfilled = 0;
    rest = 0;
    double calc = round(CurrentGauge);
    double currentMax = 12000;
    while (calc >= currentMax) {
        fulfilled += 1;
        calc -= currentMax;
        currentMax += 2000;
    }
    rest = calc / currentMax;
}

uint32_t PlayStatus::GetScore()
{
    double result = 0;
    double base = 1000000.0 / AllNotes;
    result += JusticeCritical * base * 1.01;
    result += Justice * base * 1.00;
    result += Attack * base * 0.50;
    return (uint32_t)round(result);
}

// ScoreProcessor-s -------------------------------------------

vector<shared_ptr<SusDrawableNoteData>> ScoreProcessor::DefaultDataValue;

AutoPlayerProcessor::AutoPlayerProcessor(ScenePlayer *player)
{
    Player = player;
}

void AutoPlayerProcessor::Reset()
{
    data = Player->data;
    Status.JusticeCritical = Status.Justice = Status.Attack = Status.Miss = Status.Combo = Status.CurrentGauge = 0;
    Status.AllNotes = 0;
    for (auto &note : data) {
        auto type = note->Type.to_ulong();
        if (type & SU_NOTE_LONG_MASK) {
            if (!note->Type.test((size_t)SusNoteType::AirAction)) Status.AllNotes++;
            for (auto &ex : note->ExtraData)
                if (
                    ex->Type.test((size_t)SusNoteType::End)
                    || ex->Type.test((size_t)SusNoteType::Step)
                    || ex->Type.test((size_t)SusNoteType::Injection))
                    Status.AllNotes++;
        } else if (type & SU_NOTE_SHORT_MASK) {
            Status.AllNotes++;
        }
    }
}

void AutoPlayerProcessor::Update(vector<shared_ptr<SusDrawableNoteData>> &notes)
{
    bool SlideCheck = false;
    bool HoldCheck = false;
    bool AACheck = false;
    for (auto& note : notes) {
        ProcessScore(note);
        SlideCheck = isInSlide || SlideCheck;
        HoldCheck = isInHold || HoldCheck;
        AACheck = isInAA || AACheck;
    }

    if (!wasInSlide && SlideCheck) Player->PlaySoundSlide();
    if (wasInSlide && !SlideCheck) Player->StopSoundSlide();
    if (!wasInHold && HoldCheck) Player->PlaySoundHold();
    if (wasInHold && !HoldCheck) Player->StopSoundHold();
    Player->AirActionShown = AACheck;

    wasInHold = HoldCheck;
    wasInSlide = SlideCheck;
    wasInAA = AACheck;
}

void AutoPlayerProcessor::MovePosition(double relative)
{
    double newTime = Player->CurrentSoundTime + relative;
    Status.JusticeCritical = Status.Justice = Status.Attack = Status.Miss = Status.Combo = Status.CurrentGauge = 0;

    wasInHold = isInHold = false;
    wasInSlide = isInSlide = false;
    Player->StopSoundHold();
    Player->StopSoundSlide();
    Player->RemoveSlideEffect();

    // ����: ��΂���������Finished��
    // �߂�: �����Ă��镔����Un-Finished��
    for (auto &note : data) {
        if (note->Type.test((size_t)SusNoteType::Hold)
            || note->Type.test((size_t)SusNoteType::Slide)
            || note->Type.test((size_t)SusNoteType::AirAction)) {
            if (note->StartTime <= newTime) note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
            for (auto &extra : note->ExtraData) {
                if (extra->Type.test((size_t)SusNoteType::Invisible)) continue;
                if (extra->Type.test((size_t)SusNoteType::Control)) continue;
                if (relative >= 0) {
                    if (extra->StartTime <= newTime) note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
                } else {
                    if (extra->StartTime >= newTime) note->OnTheFlyData.reset((size_t)NoteAttribute::Finished);
                }
            }
        } else {
            if (relative >= 0) {
                if (note->StartTime <= newTime) note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
            } else {
                if (note->StartTime >= newTime) note->OnTheFlyData.reset((size_t)NoteAttribute::Finished);
            }
        }
    }
}

void AutoPlayerProcessor::Draw()
{}

PlayStatus *AutoPlayerProcessor::GetPlayStatus()
{
    return &Status;
}

void AutoPlayerProcessor::ProcessScore(shared_ptr<SusDrawableNoteData> note)
{
    double relpos = (note->StartTime - Player->CurrentSoundTime) / Player->SeenDuration;
    if (relpos >= 0 || (note->OnTheFlyData.test((size_t)NoteAttribute::Finished) && note->ExtraData.size() == 0)) return;
    auto state = note->Type.to_ulong();

    if (note->Type.test((size_t)SusNoteType::Hold)) {
        isInHold = true;
        if (!note->OnTheFlyData.test((size_t)NoteAttribute::Finished)) {
            Player->PlaySoundTap();
            Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
            IncrementCombo();
            note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
        }

        for (auto &extra : note->ExtraData) {
            double pos = (extra->StartTime - Player->CurrentSoundTime) / Player->SeenDuration;
            if (pos >= 0) continue;
            if (extra->Type.test((size_t)SusNoteType::End)) isInHold = false;
            if (extra->OnTheFlyData.test((size_t)NoteAttribute::Finished)) continue;
            if (extra->Type[(size_t)SusNoteType::Injection]) {
                IncrementCombo();
                extra->OnTheFlyData.set((size_t)NoteAttribute::Finished);
                return;
            }
            Player->PlaySoundTap();
            Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
            IncrementCombo();
            extra->OnTheFlyData.set((size_t)NoteAttribute::Finished);
            return;
        }
    } else if (note->Type.test((size_t)SusNoteType::Slide)) {
        isInSlide = true;
        if (!note->OnTheFlyData.test((size_t)NoteAttribute::Finished)) {
            Player->PlaySoundTap();
            Player->SpawnSlideLoopEffect(note);

            IncrementCombo();
            note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
            return;
        }
        for (auto &extra : note->ExtraData) {
            double pos = (extra->StartTime - Player->CurrentSoundTime) / Player->SeenDuration;
            if (pos >= 0) continue;
            if (extra->Type.test((size_t)SusNoteType::End)) isInSlide = false;
            if (extra->Type.test((size_t)SusNoteType::Control)) continue;
            if (extra->Type.test((size_t)SusNoteType::Invisible)) continue;
            if (extra->OnTheFlyData.test((size_t)NoteAttribute::Finished)) continue;
            if (extra->Type.test((size_t)SusNoteType::Injection)) {
                IncrementCombo();
                extra->OnTheFlyData.set((size_t)NoteAttribute::Finished);
                return;
            }
            Player->PlaySoundTap();
            Player->SpawnJudgeEffect(extra, JudgeType::SlideTap);
            IncrementCombo();
            extra->OnTheFlyData.set((size_t)NoteAttribute::Finished);
            return;
        }
    } else if (note->Type.test((size_t)SusNoteType::AirAction)) {
        isInAA = true;
        for (auto &extra : note->ExtraData) {
            double pos = (extra->StartTime - Player->CurrentSoundTime) / Player->SeenDuration;
            if (pos >= 0) continue;
            if (extra->Type.test((size_t)SusNoteType::End)) isInAA = false;
            if (extra->Type.test((size_t)SusNoteType::Control)) continue;
            if (extra->Type.test((size_t)SusNoteType::Invisible)) continue;
            if (extra->OnTheFlyData.test((size_t)NoteAttribute::Finished)) continue;
            if (extra->Type[(size_t)SusNoteType::Injection]) {
                IncrementCombo();
                extra->OnTheFlyData.set((size_t)NoteAttribute::Finished);
                return;
            }
            if (pos >= 0) continue;
            Player->PlaySoundAirAction();
            Player->SpawnJudgeEffect(extra, JudgeType::Action);
            IncrementCombo();
            extra->OnTheFlyData.set((size_t)NoteAttribute::Finished);
        }
    } else if (note->Type.test((size_t)SusNoteType::Air)) {
        Player->PlaySoundAir();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
        Player->SpawnJudgeEffect(note, JudgeType::ShortEx);
        IncrementCombo();
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
    } else if (note->Type.test((size_t)SusNoteType::Tap)) {
        Player->PlaySoundTap();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
        IncrementCombo();
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
    } else if (note->Type.test((size_t)SusNoteType::ExTap)) {
        Player->PlaySoundExTap();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
        Player->SpawnJudgeEffect(note, JudgeType::ShortEx);
        IncrementCombo();
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
    } else if (note->Type.test((size_t)SusNoteType::Flick)) {
        Player->PlaySoundFlick();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
        IncrementCombo();
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
    } else {
        //Hell
        Player->PlaySoundTap();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
        IncrementCombo();
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
    }
}

void AutoPlayerProcessor::IncrementCombo()
{
    Status.Combo++;
    Status.JusticeCritical++;
    Status.CurrentGauge += Status.GaugeDefaultMax / Status.AllNotes;
}
