#pragma once
/**
 * SimulationParameters.h — УСТАРЕВШИЙ заголовок (сохранён для обратной совместимости).
 *
 * Новое каноническое имя согласно требованиям проекта: SimulationConfig.
 * Используйте SimulationConfig::getInstance() в новом коде.
 *
 * Этот заголовок просто перенаправляет на SimulationConfig через using-алиас,
 * поэтому весь существующий код продолжает компилироваться без изменений.
 */
#include "SimulationConfig.h"

// Алиас для обратной совместимости:
//   SimulationParameters::getInstance() → SimulationConfig::getInstance()
using SimulationParameters = SimulationConfig;
