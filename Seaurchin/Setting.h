#pragma once

//Setting���W���[��������
void InitializeSetting(HMODULE hModule);

//���[�g���擾
const std::string SettingGetRootDirectory();

//�ݒ��ǂݍ��� �Ȃ���ΐ�ɏ����o��
void SettingLoadSetting();

//�ݒ����������
void SettingSaveSetting();

//�����l�ǂݍ���
int SettingReadIntegerValue(std::string group, std::string key, int default = 0);

//�����l�ǂݍ���
double SettingReadFloatValue(std::string group, std::string key, double default = 0);

//������ǂݍ���
std::string SettingReadStringValue(std::string group, std::string key, std::string default = 0);

//�����l��������
void SettingWriteIntegerValue(std::string group, std::string key, int value);

//�����l��������
void SettingWriteFloatValue(std::string group, std::string key, double value);

//�����񏑂�����
void SettingWriteStringValue(std::string group, std::string key, std::string value);