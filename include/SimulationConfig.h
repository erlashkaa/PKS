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
    // Получить единственный экземпляр (создаётся при первом вызове, Meyers' Singleton)
    static SimulationConfig& getInstance() {
        static SimulationConfig instance;
        return instance;
    }

    // Запрещаем копирование и присваивание — Singleton должен быть в единственном числе
    SimulationConfig(const SimulationConfig&)            = delete;
    SimulationConfig& operator=(const SimulationConfig&) = delete;

    // ---- Параметры симуляции ----

    // Шаг времени одного тика (в секундах симулируемого времени)
    double tickStep = 0.001;          // по умолчанию 1 мс

    // Коэффициент времени: 1.0 = реальное время, 100.0 = в 100 раз быстрее
    double speedMultiplier = 100.0;

    // Максимальное число повторных попыток после коллизии (Binary Exponential Backoff)
    int maxRetries = 10;

    // Базовая длина слота задержки отката после коллизии (в секундах)
    // Реальное ожидание: rand(0, 2^attempt) * backoffSlot
    double backoffSlot = 0.005;       // по умолчанию 5 мс

    // Текущее модельное время (обновляется SimulationFacade на каждом тике)
    double currentTime = 0.0;

    // Флаг подробного логирования тиков в консоль
    bool enableLogs = false;

private:
    // Приватный конструктор: прямое создание экземпляра извне запрещено
    SimulationConfig() = default;
};
