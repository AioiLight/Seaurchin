#include "Controller.h"

using namespace std;

static int WacomFingerCallback(WacomMTFingerCollection *fingerPacket, void *userData);

void ControlState::Initialize()
{
    ZeroMemory(keyboardCurrent, sizeof(char) * 256);
    ZeroMemory(keyboardLast, sizeof(char) * 256);
    ZeroMemory(keyboardTrigger, sizeof(char) * 256);
    ZeroMemory(integratedSliderCurrent, sizeof(char) * 16);
    ZeroMemory(integratedSliderLast, sizeof(char) * 16);
    ZeroMemory(integratedSliderTrigger, sizeof(char) * 16);
    ZeroMemory(integratedAir, sizeof(char) * 4);

    sliderKeyboardInputCombinations[0] = { KEY_INPUT_A };
    sliderKeyboardInputCombinations[1] = { KEY_INPUT_Z };
    sliderKeyboardInputCombinations[2] = { KEY_INPUT_S };
    sliderKeyboardInputCombinations[3] = { KEY_INPUT_X };
    sliderKeyboardInputCombinations[4] = { KEY_INPUT_D };
    sliderKeyboardInputCombinations[5] = { KEY_INPUT_C };
    sliderKeyboardInputCombinations[6] = { KEY_INPUT_F };
    sliderKeyboardInputCombinations[7] = { KEY_INPUT_V };
    sliderKeyboardInputCombinations[8] = { KEY_INPUT_G };
    sliderKeyboardInputCombinations[9] = { KEY_INPUT_B };
    sliderKeyboardInputCombinations[10] = { KEY_INPUT_H };
    sliderKeyboardInputCombinations[11] = { KEY_INPUT_N };
    sliderKeyboardInputCombinations[12] = { KEY_INPUT_J };
    sliderKeyboardInputCombinations[13] = { KEY_INPUT_M };
    sliderKeyboardInputCombinations[14] = { KEY_INPUT_K };
    sliderKeyboardInputCombinations[15] = { KEY_INPUT_COMMA };
    airStringKeyboardInputCombinations[size_t(AirControlSource::AirUp)] = { KEY_INPUT_PGUP };
    airStringKeyboardInputCombinations[size_t(AirControlSource::AirDown)] = { KEY_INPUT_PGDN };
    airStringKeyboardInputCombinations[size_t(AirControlSource::AirHold)] = { KEY_INPUT_HOME };
    airStringKeyboardInputCombinations[size_t(AirControlSource::AirAction)] = { KEY_INPUT_END };

    InitializeWacomTouchDevice();
}

void ControlState::Terminate()
{
    if (isWacomDeviceAvailable) {
        WacomMTUnRegisterFingerReadCallback(wacomDeviceIds[0], nullptr, WMTProcessingModeNone, this);
        WacomMTQuit();
        UnloadWacomMTLib();
        delete[] wacomDeviceCapabilities;
        delete[] wacomDeviceIds;
    }
}

void ControlState::Update()
{
    // ���̃L�[�{�[�h����
    memcpy_s(keyboardLast, sizeof(char) * 256, keyboardCurrent, sizeof(char) * 256);
    GetHitKeyStateAll(keyboardCurrent);
    for (auto i = 0; i < 256; i++) keyboardTrigger[i] = !keyboardLast[i] && keyboardCurrent[i];

    // �L�[�{�[�h���̓X���C�_�[
    for (auto i = 0; i < 16; i++) sliderKeyboardPrevious[i] = sliderKeyboardCurrent[i];
    auto snum = 0;
    for (const auto& targets : sliderKeyboardInputCombinations) {
        auto bit = 0;
        uint32_t state = 0;
        for (const auto &knum : targets) {
            state |= (keyboardCurrent[knum] ? 1 : 0) << bit;
            ++bit;
        }
        sliderKeyboardCurrent[snum] = state;
        ++snum;
    }
    // �g���K�[�����1�ł����̓L�[��������΂悵�Ƃ���
    for (auto i = 0; i < 16; i++) sliderKeyboardTrigger[i] = sliderKeyboardCurrent[i] > sliderKeyboardPrevious[i];

    // �L�[�{�[�h���̓G�A�X�g�����O
    airStringKeyboard[size_t(AirControlSource::AirUp)] = 0;
    airStringKeyboard[size_t(AirControlSource::AirDown)] = 0;
    airStringKeyboard[size_t(AirControlSource::AirHold)] = 0;
    airStringKeyboard[size_t(AirControlSource::AirAction)] = 0;
    for (const auto &upkey : airStringKeyboardInputCombinations[size_t(AirControlSource::AirUp)]) {
        airStringKeyboard[size_t(AirControlSource::AirUp)] |= keyboardTrigger[upkey];
    }
    for (const auto &downkey : airStringKeyboardInputCombinations[size_t(AirControlSource::AirDown)]) {
        airStringKeyboard[size_t(AirControlSource::AirDown)] |= keyboardTrigger[downkey];
    }
    for (const auto &upkey : airStringKeyboardInputCombinations[size_t(AirControlSource::AirHold)]) {
        airStringKeyboard[size_t(AirControlSource::AirHold)] |= keyboardCurrent[upkey];
    }
    for (const auto &actkey : airStringKeyboardInputCombinations[size_t(AirControlSource::AirAction)]) {
        airStringKeyboard[size_t(AirControlSource::AirAction)] |= keyboardTrigger[actkey];
    }

    // ������
    for (auto i = 0; i < 16; i++) integratedSliderLast[i] = integratedSliderCurrent[i];
    for (auto i = 0; i < 16; i++) integratedSliderCurrent[i] = !!sliderKeyboardCurrent[i];
    for (auto i = 0; i < 16; i++) integratedSliderTrigger[i] = !!sliderKeyboardTrigger[i];
    integratedAir[size_t(AirControlSource::AirUp)] = airStringKeyboard[size_t(AirControlSource::AirUp)];
    integratedAir[size_t(AirControlSource::AirDown)] = airStringKeyboard[size_t(AirControlSource::AirDown)];
    integratedAir[size_t(AirControlSource::AirHold)] = airStringKeyboard[size_t(AirControlSource::AirHold)];
    integratedAir[size_t(AirControlSource::AirAction)] = airStringKeyboard[size_t(AirControlSource::AirAction)];

    /*{
        lock_guard<mutex> lock(fingerMutex);
        for (auto &finger : currentFingers) integratedSliderCurrent[finger.second->SliderPosition] = 1;
    }*/
}

bool ControlState::GetTriggerState(const ControllerSource source, const int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return keyboardTrigger[number];
        case ControllerSource::IntegratedSliders:
            if (number < 0 || number >= 16) return false;
            return integratedSliderTrigger[number];
        case ControllerSource::RawTouch:
            return false;
        case ControllerSource::IntegratedAir:
            if (number < 0 || number >= 4) return false;
            return integratedAir[number];
    }
    return false;
}

bool ControlState::GetCurrentState(const ControllerSource source, const int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return keyboardCurrent[number];
        case ControllerSource::IntegratedSliders:
            if (number < 0 || number >= 16) return false;
            return integratedSliderCurrent[number];
        case ControllerSource::RawTouch:
            return false;
        case ControllerSource::IntegratedAir:
            if (number < 0 || number >= 4) return false;
            return integratedAir[number];
    }
    return false;
}

bool ControlState::GetLastState(const ControllerSource source, const int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return keyboardLast[number];
        case ControllerSource::IntegratedSliders:
            if (number < 0 || number >= 16) return false;
            return integratedSliderLast[number];
        case ControllerSource::RawTouch:
            return false;
        case ControllerSource::IntegratedAir:
            if (number < 0 || number >= 4) return false;
            return integratedAir[number];
    }
    return false;
}

void ControlState::SetSliderKeyCombination(const int sliderNumber, const vector<int>& keys)
{
    if (sliderNumber < 0 || sliderNumber >= 16) return;
    if (keys.size() > 8) return;
    sliderKeyboardInputCombinations[sliderNumber] = keys;
}

void ControlState::SetAirStringKeyCombination(const int airNumber, const vector<int>& keys)
{
    if (airNumber < 0 || airNumber >= 4) return;
    if (keys.size() > 8) return;
    airStringKeyboardInputCombinations[airNumber] = keys;
}

void ControlState::InitializeWacomTouchDevice()
{
    auto log = spdlog::get("main");
    isWacomDeviceAvailable = false;
    log->info(u8"Wacom�^�u���b�g��0.43.0����ꎞ�I�ɋ@�\���폜���Ă��܂�");
    /*
    if (!LoadWacomMTLib()) {
        log->info(u8"Wacom�h���C�o������܂���ł���");
        return;
    }
    if (WacomMTInitialize(WACOM_MULTI_TOUCH_API_VERSION)) {
        log->warn(u8"Wacom�h���C�o�̏������Ɏ��s���܂���");
        return;
    }
    log->info(u8"Wacom�h���C�o���p�\");

    const auto devices = WacomMTGetAttachedDeviceIDs(nullptr, 0);
    if (devices <= 0) {
        log->info(u8"Wacom�f�o�C�X������܂���ł���");
        return;
    }
    wacomDeviceIds = new int[devices];
    wacomDeviceCapabilities = new WacomMTCapability[devices];
    WacomMTGetAttachedDeviceIDs(wacomDeviceIds, devices * sizeof(int));
    for (auto i = 0; i < devices; i++) {
        WacomMTCapability cap = { 0 };
        WacomMTGetDeviceCapabilities(wacomDeviceIds[i], &cap);
        wacomDeviceCapabilities[i] = cap;

        log->info(u8"�f�o�C�XID {0:2d}: {1:d}", wacomDeviceIds[i], wacomDeviceCapabilities[i].CapabilityFlags);
    }

    WacomMTRegisterFingerReadCallback(wacomDeviceIds[0], nullptr, WMTProcessingModeNone, WacomFingerCallback, this);
    isWacomDeviceAvailable = true;
    */
}

// 0.43.0�ň�U�폜�����̂ŌĂ΂�Ȃ�
void ControlState::UpdateWacomTouchDeviceFinger(WacomMTFingerCollection *fingers)
{
    const auto cap = wacomDeviceCapabilities[0];
    for (auto i = 0; i < fingers->FingerCount; i++) {
        const auto finger = fingers->Fingers[i];
        if (!finger.Confidence) continue;
        switch (finger.TouchState) {
            case WMTFingerStateNone:
                break;
            case WMTFingerStateDown: {
                lock_guard<mutex> lock(fingerMutex);
                auto data = make_shared<ControllerFingerState>();
                data->Id = finger.FingerID;
                data->State = WMTFingerStateDown;
                data->SliderPosition = floor(finger.X / cap.LogicalWidth * 16);
                currentFingers[finger.FingerID] = data;
                break;
            }
            case WMTFingerStateHold: {
                lock_guard<mutex> lock(fingerMutex);
                auto data = currentFingers[finger.FingerID];
                if (!data) {
                    auto fdata = make_shared<ControllerFingerState>();
                    fdata->Id = finger.FingerID;
                    fdata->State = WMTFingerStateDown;
                    fdata->SliderPosition = floor(finger.X / cap.LogicalWidth * 16);
                    currentFingers[finger.FingerID] = fdata;
                    break;
                }
                data->State = WMTFingerStateHold;
                data->SliderPosition = floor(finger.X / cap.LogicalWidth * 16);
                break;
            }
            case WMTFingerStateUp: {
                lock_guard<mutex> lock(fingerMutex);
                currentFingers.erase(finger.FingerID);
                break;
            }
        }
    }
}

// Wacom Multi-Touch Callbacks

// 0.43.0�ň�U�폜�����̂ŌĂ΂�Ȃ�
int WacomFingerCallback(WacomMTFingerCollection *fingerPacket, void *userData)
{
    auto controller = static_cast<ControlState*>(userData);
    controller->UpdateWacomTouchDeviceFinger(fingerPacket);
    return 0;
}