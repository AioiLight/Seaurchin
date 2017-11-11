#include "ScoreProcessor.h"
#include "ExecutionManager.h"
#include "ScenePlayer.h"
#include "Misc.h"
#include "Debug.h"

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

void AutoPlayerProcessor::IncrementCombo()
{
    Status.Combo++;
    Status.JusticeCritical++;
    Status.CurrentGauge += Status.GaugeDefaultMax / Status.AllNotes;
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

// ---------------------------------------------------
void PlayableProcessor::IncrementCombo()
{
    Status.Combo++;
    Status.JusticeCritical++;
    Status.CurrentGauge += Status.GaugeDefaultMax / Status.AllNotes;
}

void PlayableProcessor::IncrementCombo(shared_ptr<SusDrawableNoteData> note, double reltime)
{
    double jthJC = 0.033, jthJ = 0.048, jthA = 0.072, judgeAdjust = 0.020;
    if (reltime <= jthJC) {
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
        Status.JusticeCritical++;
        Status.Combo++;
        Status.CurrentGauge += Status.GaugeDefaultMax / Status.AllNotes;
    } else if (reltime <= jthJ) {
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
        Status.Justice++;
        Status.Combo++;
        Status.CurrentGauge += (Status.GaugeDefaultMax / Status.AllNotes) / 1.01;
    } else {
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
        Status.Attack++;
        Status.Combo++;
        Status.CurrentGauge += (Status.GaugeDefaultMax / Status.AllNotes) / 1.01 * 0.5;
    }
}

void PlayableProcessor::ResetCombo(shared_ptr<SusDrawableNoteData> note)
{
    note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
    Status.Miss++;
    Status.Combo = 0;
}

// TODO: �v�Z�����ꕔ�K���Ȃ�Ŏ���

void PlayableProcessor::ProcessScore(shared_ptr<SusDrawableNoteData> note)
{
    double relpos = (note->StartTime - Player->CurrentSoundTime) / Player->SeenDuration;
    if (note->OnTheFlyData.test((size_t)NoteAttribute::Finished) && note->ExtraData.size() == 0) return;
    auto state = note->Type.to_ulong();

    if (note->Type.test((size_t)SusNoteType::Hold)) {
        isInHold |= CheckHoldJudgement(note);
    } else if (note->Type.test((size_t)SusNoteType::Slide)) {
        isInSlide = CheckSlideJudgement(note);
    } else if (note->Type.test((size_t)SusNoteType::AirAction)) {
        CheckAirActionJudgement(note);
    } else if (note->Type.test((size_t)SusNoteType::Air)) {
        if (!CheckAirJudgement(note)) return;
        Player->PlaySoundAir();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
        Player->SpawnJudgeEffect(note, JudgeType::ShortEx);
    } else if (note->Type.test((size_t)SusNoteType::Tap)) {
        if (!CheckJudgement(note)) return;
        Player->PlaySoundTap();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
    } else if (note->Type.test((size_t)SusNoteType::ExTap)) {
        if (!CheckJudgement(note)) return;
        Player->PlaySoundExTap();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
        Player->SpawnJudgeEffect(note, JudgeType::ShortEx);
    } else if (note->Type.test((size_t)SusNoteType::Flick)) {
        if (!CheckJudgement(note)) return;
        Player->PlaySoundFlick();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
    } else {
        // Hell
        if (!CheckHellJudgement(note)) return;
        Player->PlaySoundTap();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
    }
}

bool PlayableProcessor::CheckHellJudgement(shared_ptr<SusDrawableNoteData> note)
{
    double jthA = 0.072, judgeAdjust = 0.020;
    double reltime = Player->CurrentTime - note->StartTime;
    if (note->OnTheFlyData.test((size_t)NoteAttribute::Finished)) return false;
    if (reltime < -jthA) return false;
    if (reltime > jthA) {
        note->OnTheFlyData.reset((size_t)NoteAttribute::HellChecking);
        note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
        return false;
    }
    if (reltime >= 0 && !note->OnTheFlyData.test((size_t)NoteAttribute::HellChecking)) {
        note->OnTheFlyData.set((size_t)NoteAttribute::HellChecking);
        Status.JusticeCritical++;
        Status.Combo++;
        Status.CurrentGauge += Status.GaugeDefaultMax / Status.AllNotes;
        return true;
    }

    for (int i = note->StartLane; i < note->StartLane + note->Length; i++) {
        if (!CurrentState->GetTriggerState(ControllerSource::IntegratedSliders, i)) continue;
        reltime = fabs(reltime);
        if (reltime <= jthA) {
            if (note->OnTheFlyData.test((size_t)NoteAttribute::HellChecking)) Status.JusticeCritical--;
            Status.Miss++;
            Status.Combo = 0;
            Status.CurrentGauge -= Status.GaugeDefaultMax / Status.AllNotes * 2;
            note->OnTheFlyData.set((size_t)NoteAttribute::Finished);
        }
        return false;
    }
    return false;
}

bool PlayableProcessor::CheckAirJudgement(shared_ptr<SusDrawableNoteData> note)
{
    double jthJC = 0.033, jthJ = 0.048, jthA = 0.072, judgeAdjust = 0.020;
    double reltime = Player->CurrentTime - note->StartTime + judgeAdjust;
    if (note->OnTheFlyData.test((size_t)NoteAttribute::Finished)) return false;
    if (reltime < -jthA) return false;
    if (reltime > jthA) {
        ResetCombo(note);
        return false;
    }
    if (note->Type[(size_t)SusNoteType::Up]) {
        if (!CurrentState->GetTriggerState(ControllerSource::IntegratedAir, (int)AirControlSource::AirUp)) return false;
    } else {
        if (!CurrentState->GetTriggerState(ControllerSource::IntegratedAir, (int)AirControlSource::AirDown)) return false;
    }
    reltime = fabs(reltime);
    IncrementCombo(note, reltime);
    return true;
}

bool PlayableProcessor::CheckAirActionJudgement(shared_ptr<SusDrawableNoteData> note)
{
    double jthJC = 0.033, jthJ = 0.048, jthA = 0.072, judgeAdjust = 0.020;
    double reltime = Player->CurrentTime - note->StartTime + judgeAdjust;
    if (reltime < 0) return false;
    if (note->OnTheFlyData[(size_t)NoteAttribute::Completed]) return false;

    if (!note->OnTheFlyData[(size_t)NoteAttribute::Finished]) {
        if (CurrentState->GetCurrentState(ControllerSource::IntegratedAir, (int)AirControlSource::AirHold)) {
            IncrementCombo(note, reltime);
            return true;
        }
        if (reltime >= jthA) {
            ResetCombo(note);
            return false;
        }
    }
    for (const auto &extra : note->ExtraData) {
        reltime = Player->CurrentSoundTime - extra->StartTime + judgeAdjust;
        if (extra->OnTheFlyData[(size_t)NoteAttribute::Finished]) continue;
        if (extra->Type[(size_t)SusNoteType::Control]) continue;
        if (extra->Type[(size_t)SusNoteType::Invisible]) continue;
        if (extra->Type[(size_t)SusNoteType::Injection] && reltime >= 0) {
            if (CurrentState->GetCurrentState(ControllerSource::IntegratedAir, (int)AirControlSource::AirHold)) {
                IncrementCombo(extra, reltime);
                return true;
            }
        }
        if (reltime < -jthA) return false;
        if (reltime >= jthA) {
            ResetCombo(extra);
            return false;
        }
        if (!CurrentState->GetTriggerState(ControllerSource::IntegratedAir, (int)AirControlSource::AirAction)) continue;
        reltime = fabs(reltime);
        IncrementCombo(extra, reltime);
        Player->PlaySoundAirAction();
        Player->SpawnJudgeEffect(extra, JudgeType::Action);
        if (extra->Type[(size_t)SusNoteType::End]) note->OnTheFlyData.set((size_t)NoteAttribute::Completed);
        return true;
    }
    return false;
}

bool PlayableProcessor::CheckHoldJudgement(shared_ptr<SusDrawableNoteData> note)
{
    double jthJC = 0.033, jthJ = 0.048, jthA = 0.072, judgeAdjust = 0.020;
    double reltime = Player->CurrentTime - note->StartTime + judgeAdjust;
    if (reltime < -jthA) return false;
    if (note->OnTheFlyData[(size_t)NoteAttribute::Completed]) return false;

    bool intersect = false;
    for (int i = note->StartLane; i < (note->StartLane + note->Length); i++) intersect |= CurrentState->GetTriggerState(ControllerSource::IntegratedSliders, i);

    if (!note->OnTheFlyData[(size_t)NoteAttribute::Finished]) {
        if (intersect) {
            IncrementCombo(note, reltime);
            Player->PlaySoundTap();
            Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
            return intersect;
        }
        if (reltime >= jthA) {
            ResetCombo(note);
            return intersect;
        }
    }
    for (int i = note->StartLane; i < (note->StartLane + note->Length); i++) intersect |= CurrentState->GetCurrentState(ControllerSource::IntegratedSliders, i);
    for (const auto &extra : note->ExtraData) {
        reltime = Player->CurrentSoundTime - extra->StartTime + judgeAdjust;
        if (extra->OnTheFlyData[(size_t)NoteAttribute::Finished]) continue;
        if (reltime < 0) return intersect;
        if (reltime >= jthA) {
            ResetCombo(extra);
            return intersect;
        }

        if (!intersect) return intersect;

        reltime = fabs(reltime);
        if (extra->Type[(size_t)SusNoteType::Injection]) {
            IncrementCombo(extra, reltime);
            return intersect;
        }

        IncrementCombo(extra, reltime);
        Player->PlaySoundTap();
        Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
        if (extra->Type[(size_t)SusNoteType::End]) note->OnTheFlyData.set((size_t)NoteAttribute::Completed);
        return intersect;
    }
    return intersect;
}

bool PlayableProcessor::CheckSlideJudgement(shared_ptr<SusDrawableNoteData> note)
{
    double jthJC = 0.033, jthJ = 0.048, jthA = 0.072, judgeAdjust = 0.020;
    double reltime = Player->CurrentTime - note->StartTime + judgeAdjust;
    if (reltime < -jthA) return false;
    if (note->OnTheFlyData[(size_t)NoteAttribute::Completed]) return false;

    bool intersect = false;
    for (int i = note->StartLane; i < (note->StartLane + note->Length); i++) intersect |= CurrentState->GetTriggerState(ControllerSource::IntegratedSliders, i);

    if (!note->OnTheFlyData[(size_t)NoteAttribute::Finished]) {
        if (intersect) {
            IncrementCombo(note, reltime);
            Player->PlaySoundTap();
            Player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
            return intersect;
        }
        if (reltime >= jthA) {
            ResetCombo(note);
            return intersect;
        }
    }
    //���݂̔���ʒu�𒲂ׂ�
    auto lastStep = note;
    auto refNote = note;
    for (const auto &extra : note->ExtraData) {
        if (extra->Type[(size_t)SusNoteType::Control]) continue;
        if (extra->Type[(size_t)SusNoteType::Injection]) continue;
        if (Player->CurrentTime <= extra->StartTime) {
            refNote = extra;
            break;
        }
        lastStep = refNote = extra;
    }
    if (lastStep == refNote) return intersect;
    auto &refcurve = Player->curveData[refNote];
    double timeInBlock = Player->CurrentTime - lastStep->StartTime;
    if (timeInBlock < 0) return intersect;
    auto start = refcurve[0];
    auto next = refcurve[0];
    for (const auto &segment : refcurve) {
        if (get<0>(segment) >= timeInBlock) {
            next = segment;
            break;
        }
        start = next = segment;
    }
    auto center = lerp((timeInBlock - get<0>(start)) / (get<0>(next) - get<0>(start)), get<1>(start), get<1>(next)) * 16;
    auto width = lerp(timeInBlock / (refNote->StartTime - lastStep->StartTime), lastStep->Length, refNote->Length);
    int left = floor(center - width / 2.0);
    int right = ceil(center + width / 2.0);

    for (int i = left; i < right; i++) intersect |= CurrentState->GetCurrentState(ControllerSource::IntegratedSliders, i);
    for (const auto &extra : note->ExtraData) {
        reltime = Player->CurrentSoundTime - extra->StartTime + judgeAdjust;
        if (extra->OnTheFlyData[(size_t)NoteAttribute::Finished]) continue;
        if (extra->Type[(size_t)SusNoteType::Control]) continue;
        if (extra->Type[(size_t)SusNoteType::Invisible]) continue;
        if (reltime < 0) return intersect;
        if (reltime >= jthA) {
            ResetCombo(extra);
            return intersect;
        }

        if (!intersect) return intersect;

        reltime = fabs(reltime);
        if (extra->Type[(size_t)SusNoteType::Injection]) {
            IncrementCombo(extra, reltime);
            return intersect;
        }

        IncrementCombo(extra, reltime);
        Player->PlaySoundTap();
        Player->SpawnJudgeEffect(extra, JudgeType::SlideTap);
        if (extra->Type[(size_t)SusNoteType::End]) note->OnTheFlyData.set((size_t)NoteAttribute::Completed);
        return intersect;
    }
    return intersect;
}


bool PlayableProcessor::CheckJudgement(shared_ptr<SusDrawableNoteData> note)
{
    double jthJC = 0.033, jthJ = 0.048, jthA = 0.072, judgeAdjust = 0.020;
    double reltime = Player->CurrentTime - note->StartTime + judgeAdjust;
    if (note->OnTheFlyData.test((size_t)NoteAttribute::Finished)) return false;
    if (reltime < -jthA) return false;
    if (reltime > jthA) {
        ResetCombo(note);
        return false;
    }
    for (int i = note->StartLane; i < note->StartLane + note->Length; i++) {
        if (!CurrentState->GetTriggerState(ControllerSource::IntegratedSliders, i)) continue;
        reltime = fabs(reltime);
        IncrementCombo(note, reltime);
        return true;
    }
    return false;
}

PlayableProcessor::PlayableProcessor(ScenePlayer * player)
{
    Player = player;
    CurrentState = Player->manager->GetControlStateSafe();
}

void PlayableProcessor::Reset()
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

    imageHoldLight = dynamic_cast<SImage*>(Player->resources["LaneHoldLight"]);
}

void PlayableProcessor::Update(vector<shared_ptr<SusDrawableNoteData>>& notes)
{
    bool SlideCheck = false;
    bool HoldCheck = false;
    isInHold = false;
    for (auto& note : notes) {
        ProcessScore(note);
        SlideCheck = isInSlide || SlideCheck;
        HoldCheck = isInHold || HoldCheck;
    }

    if (!wasInSlide && SlideCheck) Player->PlaySoundSlide();
    if (wasInSlide && !SlideCheck) Player->StopSoundSlide();
    if (!wasInHold && HoldCheck) Player->PlaySoundHold();
    if (wasInHold && !HoldCheck) Player->StopSoundHold();

    wasInHold = HoldCheck;
    wasInSlide = SlideCheck;
}

void PlayableProcessor::MovePosition(double relative)
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

void PlayableProcessor::Draw()
{
    if (!imageHoldLight) return;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    for (int i = 0; i < 16; i++)
        if (CurrentState->GetCurrentState(ControllerSource::IntegratedSliders, i))
            DrawRectRotaGraph3F(
                Player->widthPerLane * i, Player->laneBufferY,
                0, 0,
                imageHoldLight->get_Width(), imageHoldLight->get_Height(),
                0, imageHoldLight->get_Height(),
                1, 2, 0,
                imageHoldLight->GetHandle(), TRUE, FALSE);
}

PlayStatus *PlayableProcessor::GetPlayStatus()
{
    return &Status;
}
