#pragma once

#define SU_SETTING_GENERAL "General"
#define SU_SETTING_SKIN "Skin"

class Setting final {
private:
    static std::wstring RootDirectory;
    std::wstring file;
    toml::Value SettingTree;

public:
    Setting();
    Setting(HMODULE hModule);
    void Load(const std::wstring &filename);
    inline void Reload() { Load(file); }
    void Save() const;
    static const std::wstring GetRootDirectory();

    template<typename T>
    T ReadValue(const std::string &group, const std::string &key, T defValue)
    {
        auto v = SettingTree.find(group + "." + key);
        if (v && v->is<T>()) {
            return v->as<T>();
        } else {
            WriteValue(group, key, defValue);
            return defValue;
        }
    }

    template<typename T>
    void WriteValue(const std::string &group, const std::string &key, T value)
    {
        SettingTree.set(group + "." + key, value);
    }
};

enum SettingType {
    IntegerSetting,
    FloatSetting,
    StringSetting,
    BooleanSetting,
};

class SettingItem {
protected:
    std::shared_ptr<Setting> SettingInstance;
    std::string SettingGroup;
    std::string SettingKey;
    std::string Description;
    SettingType Type;

public:
    SettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);

    SettingType GetType() { return Type; }
    void SetDescription(const std::string &desc) { Description = desc; }
    std::string GetDescription() { return Description; }
    virtual std::string GetItemString() = 0;
    virtual void MoveNext() = 0;
    virtual void MovePrevious() = 0;
    virtual void SaveValue() = 0;
    virtual void RetrieveValue() = 0;
};

class NumberSettingItem : public SettingItem {
protected:
    double Value = 0;
    double Step = 1;
    int FloatDigits = 0;
    double MinValue = 0;
    double MaxValue = 100;
    std::function<std::string(NumberSettingItem*)> Formatter;

    static std::function<std::string(NumberSettingItem*)> DefaultFormatter;

public:
    NumberSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);

    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void SetStep(double step);
    void SetRange(double min, double max);
    void SetFloatDigits(int digits);
    int GetFloatDigits() { return FloatDigits; }
    double GetValue() { return Value; }
};

class BooleanSettingItem : public SettingItem {
protected:
    bool Value = false;
    std::string FalseText = "False";
    std::string TrueText = "True";

public:
    BooleanSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);

    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void SetText(const std::string &t, const std::string &f);
    bool GetValue() { return Value; }
};

class SettingItemManager final {
private:
    std::shared_ptr<Setting> ReferredSetting;
    std::unordered_map<std::string, std::shared_ptr<SettingItem>> Items;

public:
    SettingItemManager(std::shared_ptr<Setting> setting);

    void AddSettingByString(const std::string &proc);
    std::shared_ptr<SettingItem> GetSettingItem(const std::string &group, const std::string &key);
    void RetrieveAllValues();
    void SaveAllValues();
};