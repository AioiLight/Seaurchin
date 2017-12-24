#include "SkinHolder.h"
#include "Setting.h"
#include "ExecutionManager.h"

using namespace std;
using namespace boost::filesystem;

bool SkinHolder::IncludeScript(std::wstring include, std::wstring from, CWScriptBuilder * builder)
{
    return false;
}

SkinHolder::SkinHolder(const wstring &name, shared_ptr<AngelScript> script, std::shared_ptr<SoundManager> sound)
{
    ScriptInterface = script;
	SoundInterface = sound;
    SkinName = name;
    SkinRoot = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR / SkinName;
}

SkinHolder::~SkinHolder()
{

}

void SkinHolder::Initialize()
{
    auto log = spdlog::get("main");
    ScriptInterface->StartBuildModule("SkinLoader",
        [this](wstring inc, wstring from, CWScriptBuilder *b)
    {
        if (!exists(SkinRoot / SU_SCRIPT_DIR / inc)) return false;
        b->AddSectionFromFile((SkinRoot / SU_SCRIPT_DIR / inc).wstring().c_str());
        return true;
    });
    ScriptInterface->LoadFile((SkinRoot / SU_SKIN_MAIN_FILE).wstring().c_str());
    ScriptInterface->FinishBuildModule();

    auto mod = ScriptInterface->GetLastModule();
    int fc = mod->GetFunctionCount();
    asIScriptFunction *ep = nullptr;
    for (asUINT i = 0; i < fc; i++)
    {
        auto func = mod->GetFunctionByIndex(i);
        if (!ScriptInterface->CheckMetaData(func, "EntryPoint")) continue;
        ep = func;
        break;
    }
    if (!ep)
    {
        log->critical(u8"�X�L����EntryPoint������܂���");
        mod->Discard();
        return;
    }

    auto ctx = ScriptInterface->GetEngine()->CreateContext();
    ctx->Prepare(ep);
    ctx->SetArgObject(0, this);
    ctx->Execute();
    ctx->Release();
    mod->Discard();
}

void SkinHolder::Terminate()
{
    for (const auto &it : Images) it.second->Release();
    for (const auto &it : Sounds) it.second->Release();
    for (const auto &it : Fonts) it.second->Release();
    for (const auto &it : AnimatedImages) it.second->Release();
}

asIScriptObject* SkinHolder::ExecuteSkinScript(const wstring &file)
{
    auto log = spdlog::get("main");
    //���������
    auto modulename = ConvertUnicodeToUTF8(file);
    auto mod = ScriptInterface->GetExistModule(modulename);
    if (!mod)
    {
        ScriptInterface->StartBuildModule(modulename.c_str(),
            [this](wstring inc, wstring from, CWScriptBuilder *b)
        {
            if (!exists(SkinRoot / SU_SCRIPT_DIR / inc)) return false;
            b->AddSectionFromFile((SkinRoot / SU_SCRIPT_DIR / inc).wstring().c_str());
            return true;
        });
        ScriptInterface->LoadFile((SkinRoot / SU_SCRIPT_DIR / file).wstring().c_str());
        if (!ScriptInterface->FinishBuildModule()) {
            ScriptInterface->GetLastModule()->Discard();
            return nullptr;
        }
        mod = ScriptInterface->GetLastModule();
    }

    //�G���g���|�C���g����
    int cnt = mod->GetObjectTypeCount();
    asITypeInfo *type = nullptr;
    for (int i = 0; i < cnt; i++)
    {
        // ScriptBuilder��MetaData�̃e�[�u���͖���j�������̂�
        // asITypeInfo�ɏ���ێ�
        auto cti = mod->GetObjectTypeByIndex(i);
        if (!(ScriptInterface->CheckMetaData(cti, "EntryPoint") || cti->GetUserData(SU_UDTYPE_ENTRYPOINT))) continue;
        type = cti;
        type->SetUserData((void*)0xFFFFFFFF, SU_UDTYPE_ENTRYPOINT);
        type->AddRef();
        break;
    }
    if (!type)
    {
        log->critical(u8"�X�L����EntryPoint������܂���");
        return nullptr;
    }

    auto obj = ScriptInterface->InstantiateObject(type);
    obj->SetUserData(this, SU_UDTYPE_SKIN);
    type->Release();
    return obj;
}

void SkinHolder::LoadSkinImage(const string &key, const string &filename)
{
    Images[key] = SImage::CreateLoadedImageFromFile(ConvertUnicodeToUTF8((SkinRoot / SU_IMAGE_DIR / ConvertUTF8ToUnicode(filename)).wstring()), false);
}

void SkinHolder::LoadSkinFont(const string &key, const string &filename)
{
    Fonts[key] = SFont::CreateLoadedFontFromFile(ConvertUnicodeToUTF8((SkinRoot / SU_FONT_DIR / ConvertUTF8ToUnicode(filename)).wstring()));
}

void SkinHolder::LoadSkinSound(const std::string & key, const std::string & filename)
{
	Sounds[key] = SSound::CreateSoundFromFile(SoundInterface.get(), ConvertUnicodeToUTF8((SkinRoot / SU_SOUND_DIR / ConvertUTF8ToUnicode(filename)).wstring()), 1);
}

void SkinHolder::LoadSkinAnime(const std::string & key, const std::string & filename, int x, int y, int w, int h, int c, double time)
{
    AnimatedImages[key] = SAnimatedImage::CreateLoadedImageFromFile(ConvertUnicodeToUTF8((SkinRoot / SU_IMAGE_DIR / ConvertUTF8ToUnicode(filename)).wstring()), x, y, w, h, c, time);
}

SImage* SkinHolder::GetSkinImage(const string &key)
{
    auto it = Images.find(key);
    if (it == Images.end()) return nullptr;
    it->second->AddRef();
    return it->second;
}

SFont* SkinHolder::GetSkinFont(const string &key)
{
    auto it = Fonts.find(key);
    if (it == Fonts.end()) return nullptr;
    it->second->AddRef();
    return it->second;
}

SSound* SkinHolder::GetSkinSound(const std::string & key)
{
	auto it = Sounds.find(key);
	if (it == Sounds.end()) return nullptr;
	it->second->AddRef();
	return it->second;
}

SAnimatedImage * SkinHolder::GetSkinAnime(const std::string & key)
{
    auto it = AnimatedImages.find(key);
    if (it == AnimatedImages.end()) return nullptr;
    it->second->AddRef();
    return it->second;
}

void RegisterScriptSkin(ExecutionManager *exm)
{
	auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

    engine->RegisterObjectType(SU_IF_SKIN, 0, asOBJ_REF | asOBJ_NOCOUNT);
    engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadImage(const string &in, const string &in)", asMETHOD(SkinHolder, LoadSkinImage), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadFont(const string &in, const string &in)", asMETHOD(SkinHolder, LoadSkinFont), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadSound(const string &in, const string &in)", asMETHOD(SkinHolder, LoadSkinSound), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadAnime(const string &in, const string &in, int, int, int, int, int, double)", asMETHOD(SkinHolder, LoadSkinAnime), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SKIN, SU_IF_IMAGE "@ GetImage(const string &in)", asMETHOD(SkinHolder, GetSkinImage), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SKIN, SU_IF_FONT "@ GetFont(const string &in)", asMETHOD(SkinHolder, GetSkinFont), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, SU_IF_SOUND "@ GetSound(const string &in)", asMETHOD(SkinHolder, GetSkinSound), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SKIN, SU_IF_ANIMEIMAGE "@ GetAnime(const string &in)", asMETHOD(SkinHolder, GetSkinAnime), asCALL_THISCALL);

    engine->RegisterGlobalFunction(SU_IF_SKIN "@ GetSkin()", asFUNCTION(GetSkinObject), asCALL_CDECL);
}

//�X�L����p
SkinHolder* GetSkinObject()
{
    auto ctx = asGetActiveContext();
    auto obj = (asIScriptObject*)ctx->GetThisPointer();
    if (!obj)
    {
        ScriptSceneWarnOutOf("Instance Method", ctx);
        return nullptr;
    }
    auto skin = obj->GetUserData(SU_UDTYPE_SKIN);
    if (!skin)
    {
        ScriptSceneWarnOutOf("Skin-Related Scene", ctx);
        return nullptr;
    }
    return (SkinHolder*)skin;
}