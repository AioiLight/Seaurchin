#pragma once

#include <Windows.h>
#include <Shlwapi.h>

#include <stdio.h>

#include <ios>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define ROOT_FILE(dir, fn) (dir + "/" + fn)

#define SU_SETTING_FILE "config.json"

//Setting���W���[��������
void InitializeSetting(HMODULE hModule);

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