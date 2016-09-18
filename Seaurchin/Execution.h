#pragma once

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/xpressive/xpressive.hpp>

#include "SceneManager.h"

//�X�L���̗� �K�{
void ExecutionEnumerateSkins();

//�����J�n����
void ExecutionExecute();
//Tick
void ExecutionTick(double delta);
//Draw
void ExecutionDraw();

void ExecutionAddScene(std::shared_ptr<Scene> scene);
std::shared_ptr<SceneManager> ExecutionGetManager();
