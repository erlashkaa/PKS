#pragma once

/**
 * SimulationConfig — глобальные параметры среды симуляции (паттерн Singleton).
 *
 * Singleton гарантирует, что существует ровно ОДИН экземпляр этого класса.
 * Все модули симулятора обращаются к единому объекту через getInstance().
 *
 * Хранит:
 *  - коэффициент времени (speedMultiplier)
 *  - шаг симуляции (tickStep)
 *  - лимиты задержек (maxRetries, backoffSlot)
 *  - текущее модельное время
 *
 * Использование:
 *   auto& cfg = SimulationConfig::getInstance();
 *   cfg.tickStep = 0.001;
 */
class SimulationConfig {
public:
    static SimulationConfig& getInstance() {
        static SimulationConfig instance;
        return instance;
    }
    SimulationConfig(const SimulationConfig&)            = delete;
    SimulationConfig& operator=(const SimulationConfig&) = delete;
    double tickStep = 0.001;  
    double speedMultiplier = 100.0;
    int maxRetries = 10;
    double backoffSlot = 0.005;
    double currentTime = 0.0;
    bool enableLogs = false;

private:
    SimulationConfig() = default;
};
