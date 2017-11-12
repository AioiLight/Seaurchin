#include "Controller.h"
#include "Debug.h"

using namespace std;

static int WacomFingerCallback(WacomMTFingerCollection *fingerPacket, void *userData);

void ControlState::Initialize()
{
    ZeroMemory(KeyboardCurrent, sizeof(char) * 256);
    ZeroMemory(KeyboardLast, sizeof(char) * 256);
    ZeroMemory(KeyboardTrigger, sizeof(char) * 256);
    ZeroMemory(IntegratedSliderCurrent, sizeof(char) * 16);
    ZeroMemory(IntegratedSliderLast, sizeof(char) * 16);
    ZeroMemory(IntegratedSliderTrigger, sizeof(char) * 16);
    ZeroMemory(IntegratedAir, sizeof(char) * 4);

    SliderKeyboardNumbers[0] = KEY_INPUT_A;
    SliderKeyboardNumbers[1] = KEY_INPUT_Z;
    SliderKeyboardNumbers[2] = KEY_INPUT_S;
    SliderKeyboardNumbers[3] = KEY_INPUT_X;
    SliderKeyboardNumbers[4] = KEY_INPUT_D;
    SliderKeyboardNumbers[5] = KEY_INPUT_C;
    SliderKeyboardNumbers[6] = KEY_INPUT_F;
    SliderKeyboardNumbers[7] = KEY_INPUT_V;
    SliderKeyboardNumbers[8] = KEY_INPUT_G;
    SliderKeyboardNumbers[9] = KEY_INPUT_B;
    SliderKeyboardNumbers[10] = KEY_INPUT_H;
    SliderKeyboardNumbers[11] = KEY_INPUT_N;
    SliderKeyboardNumbers[12] = KEY_INPUT_J;
    SliderKeyboardNumbers[13] = KEY_INPUT_M;
    SliderKeyboardNumbers[14] = KEY_INPUT_K;
    SliderKeyboardNumbers[15] = KEY_INPUT_COMMA;
    AirStringKeyboardNumbers[(size_t)AirControlSource::AirUp] = KEY_INPUT_PGUP;
    AirStringKeyboardNumbers[(size_t)AirControlSource::AirDown] = KEY_INPUT_PGDN;
    AirStringKeyboardNumbers[(size_t)AirControlSource::AirHold] = KEY_INPUT_HOME;
    AirStringKeyboardNumbers[(size_t)AirControlSource::AirAction] = KEY_INPUT_END;

    InitializeWacomTouchDevice();
}

void ControlState::Terminate()
{
    if (IsWacomDeviceAvailable) {
        WacomMTUnRegisterFingerReadCallback(WacomDeviceIds[0], NULL, WacomMTProcessingMode::WMTProcessingModeNone, this);
        WacomMTQuit();
        UnloadWacomMTLib();
        delete[] WacomDeviceCapabilities;
        delete[] WacomDeviceIds;
    }
}

void ControlState::Update()
{
    memcpy_s(KeyboardLast, sizeof(char) * 256, KeyboardCurrent, sizeof(char) * 256);
    GetHitKeyStateAll(KeyboardCurrent);
    for (int i = 0; i < 256; i++) KeyboardTrigger[i] = !KeyboardLast[i] && KeyboardCurrent[i];

    for (int i = 0; i < 16; i++) IntegratedSliderLast[i] = IntegratedSliderCurrent[i];
    for (int i = 0; i < 16; i++) IntegratedSliderCurrent[i] = KeyboardCurrent[SliderKeyboardNumbers[i]];
    IntegratedAir[(size_t)AirControlSource::AirUp] = KeyboardTrigger[AirStringKeyboardNumbers[(size_t)AirControlSource::AirUp]];
    IntegratedAir[(size_t)AirControlSource::AirDown] = KeyboardTrigger[AirStringKeyboardNumbers[(size_t)AirControlSource::AirDown]];
    IntegratedAir[(size_t)AirControlSource::AirHold] = KeyboardCurrent[AirStringKeyboardNumbers[(size_t)AirControlSource::AirHold]];
    IntegratedAir[(size_t)AirControlSource::AirAction] = KeyboardTrigger[AirStringKeyboardNumbers[(size_t)AirControlSource::AirAction]];

    {
        lock_guard<mutex> lock(FingerMutex);
        for (auto &finger : CurrentFingers) IntegratedSliderCurrent[finger.second->SliderPosition] = 1;
    }

    for (int i = 0; i < 16; i++) IntegratedSliderTrigger[i] = !IntegratedSliderLast[i] && IntegratedSliderCurrent[i];
}

bool ControlState::GetTriggerState(ControllerSource source, int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return KeyboardTrigger[number];
        case ControllerSource::IntegratedSliders:
            if (number < 0 || number >= 16) return false;
            return IntegratedSliderTrigger[number];
        case ControllerSource::RawTouch:
            return false;
        case ControllerSource::IntegratedAir:
            if (number < 0 || number >= 4) return false;
            return IntegratedAir[number];
    }
    return false;
}

bool ControlState::GetCurrentState(ControllerSource source, int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return KeyboardCurrent[number];
        case ControllerSource::IntegratedSliders:
            if (number < 0 || number >= 16) return false;
            return IntegratedSliderCurrent[number];
        case ControllerSource::RawTouch:
            return false;
        case ControllerSource::IntegratedAir:
            if (number < 0 || number >= 4) return false;
            return IntegratedAir[number];
    }
    return false;
}

bool ControlState::GetLastState(ControllerSource source, int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return KeyboardLast[number];
        case ControllerSource::IntegratedSliders:
            if (number < 0 || number >= 16) return false;
            return IntegratedSliderLast[number];
        case ControllerSource::RawTouch:
            return false;
        case ControllerSource::IntegratedAir:
            if (number < 0 || number >= 4) return false;
            return IntegratedAir[number];
    }
    return false;
}

void ControlState::SetSliderKey(int sliderNumber, int keyboardNumber)
{
    if (sliderNumber < 0 || sliderNumber >= 16) return;
    if (keyboardNumber < 0 || keyboardNumber >= 256) return;
    SliderKeyboardNumbers[sliderNumber] = keyboardNumber;
}

void ControlState::InitializeWacomTouchDevice()
{
    IsWacomDeviceAvailable = false;
    if (!LoadWacomMTLib()) {
        WriteDebugConsole("Wacom Touch Library Unavailable.\n");
        return;
    }
    if (WacomMTInitialize(WACOM_MULTI_TOUCH_API_VERSION)) {
        WriteDebugConsole("Failed to Initialize Wacom Touch Library.\n");
        return;
    }
    WriteDebugConsole("Wacom Touch Device Available.\n");

    int devices = WacomMTGetAttachedDeviceIDs(nullptr, 0);
    if (devices <= 0) {
        WriteDebugConsole("Attached Device Not Found.\n");
        return;
    }
    WacomDeviceIds = new int[devices];
    WacomDeviceCapabilities = new WacomMTCapability[devices];
    WacomMTGetAttachedDeviceIDs(WacomDeviceIds, devices * sizeof(int));
    for (int i = 0; i < devices; i++) {
        WacomMTCapability cap = { 0 };
        WacomMTGetDeviceCapabilities(WacomDeviceIds[i], &cap);
        WacomDeviceCapabilities[i] = cap;

        ostringstream ss;
        ss << "Device Id " << WacomDeviceIds[i] << ": " << WacomDeviceCapabilities[i].CapabilityFlags << endl;
        WriteDebugConsole(ss.str().c_str());
    }

    WacomMTRegisterFingerReadCallback(WacomDeviceIds[0], nullptr, WacomMTProcessingMode::WMTProcessingModeNone, WacomFingerCallback, this);
    IsWacomDeviceAvailable = true;
}

void ControlState::UpdateWacomTouchDeviceFinger(WacomMTFingerCollection *fingers)
{
    auto cap = WacomDeviceCapabilities[0];
    for (int i = 0; i < fingers->FingerCount; i++) {
        auto finger = fingers->Fingers[i];
        if (!finger.Confidence) continue;
        switch (finger.TouchState) {
            case WacomMTFingerState::WMTFingerStateNone:
                break;
            case WacomMTFingerState::WMTFingerStateDown: {
                lock_guard<mutex> lock(FingerMutex);
                auto data = make_shared<ControllerFingerState>();
                data->Id = finger.FingerID;
                data->State = WacomMTFingerState::WMTFingerStateDown;
                data->SliderPosition = floor(finger.X / cap.LogicalWidth * 16);
                CurrentFingers[finger.FingerID] = data;

                //ostringstream ss;
                //ss << "Touched #" << finger.FingerID << " at " << data->SliderPosition << endl;
                //WriteDebugConsole(ss.str().c_str());
                break;
            }
            case WacomMTFingerState::WMTFingerStateHold: {
                lock_guard<mutex> lock(FingerMutex);
                auto data = CurrentFingers[finger.FingerID];
                if (!data) {
                    auto data = make_shared<ControllerFingerState>();
                    data->Id = finger.FingerID;
                    data->State = WacomMTFingerState::WMTFingerStateDown;
                    data->SliderPosition = floor(finger.X / cap.LogicalWidth * 16);
                    CurrentFingers[finger.FingerID] = data;
                    break;
                }
                data->State = WacomMTFingerState::WMTFingerStateHold;
                data->SliderPosition = floor(finger.X / cap.LogicalWidth * 16);
                break;
            }
            case WacomMTFingerState::WMTFingerStateUp: {
                lock_guard<mutex> lock(FingerMutex);
                CurrentFingers.erase(finger.FingerID);
                break;
            }
        }
    }
}

// Wacom Multi-Touch Callbacks

int WacomFingerCallback(WacomMTFingerCollection *fingerPacket, void *userData)
{
    auto controller = static_cast<ControlState*>(userData);
    controller->UpdateWacomTouchDeviceFinger(fingerPacket);
    return 0;
}