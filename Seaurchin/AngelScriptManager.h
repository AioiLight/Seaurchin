#pragma once

typedef std::function<bool(std::string, std::string, CScriptBuilder*)> IncludeCallback;

class AngelScript
{
private:
    asIScriptEngine *engine;
    asIScriptContext *sharedContext;
    CScriptBuilder builder;
    IncludeCallback includeFunc;

    void ScriptMessageCallback(const asSMessageInfo *message);

public:
    AngelScript();
    ~AngelScript();

    inline asIScriptEngine* GetEngine() { return engine; }
    inline asIScriptContext* GetContext() { return sharedContext; }

    //�V����Module����
    void StartBuildModule(const std::string &name, IncludeCallback callback);

    inline asIScriptModule* GetExistModule(std::string name) { return engine->GetModule(name.c_str()); }
    
    //�t�@�C���ǂݍ���
    void LoadFile(const std::wstring &filename);
    
    //�O����g��Ȃ���
    bool IncludeFile(const std::wstring &include, const std::wstring &from);
    
    //�r���h����
    bool FinishBuildModule();
    
    //Finish����Module���擾
    inline asIScriptModule* GetLastModule() { return builder.GetModule(); }

    //����N���X�Ƀ��^�f�[�^���t�^����Ă邩
    bool CheckMetaData(asITypeInfo *type, std::string meta);
    
    //����O���ւ�(ry
    bool CheckMetaData(asIScriptFunction *type, std::string meta);


    //�������`�F�b�N
    inline bool CheckImplementation(asITypeInfo *type, std::string name)
    {
        return type->Implements(engine->GetTypeInfoByName(name.c_str()));
    }
    
    //asITypeInfo����C���X�^���X�쐬 ���t�@�����X�����Ȃ̂ł�������AddRef����
    asIScriptObject* InstantiateObject(asITypeInfo *type);
};