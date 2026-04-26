#pragma once

/**
 * SimulationParameters — глобальные настройки симуляции (паттерн Singleton).
 *
 * Singleton гарантирует, что существует ровно ОДИН экземпляр этого класса.
 * Все модули симулятора обращаются к единому объекту через getInstance().
 *
 * Хранит:
 *  - шаг симуляции (tickStep)
 *  - множитель скорости (speedMultiplier)
 *  - максимальное число повторных попыток CSMA/CD после коллизии
 *  - базовая задержка отката после коллизии (backoff)
 */
class SimulationParameters {
public:
    // Получить единственный экземпляр (создаётся при первом вызове)
    static SimulationParameters& getInstance() {
        static SimulationParameters instance; // Гарантировано создаётся один раз
        return instance;
    }

    // Удаляем конструктор копирования и оператор присваивания,
    // чтобы предотвратить создание дополнительных копий.
    SimulationParameters(const SimulationParameters&)            = delete;
    SimulationParameters& operator=(const SimulationParameters&) = delete;

    // ---- Параметры симуляции ----

    // Шаг времени одного тика (в секундах симулируемого времени)
    double tickStep = 0.001; // 1 мс

    // Множитель скорости: 1.0 = реальное время, 10.0 = в 10 раз быстрее
    double speedMultiplier = 100.0;

    // Максимальное число повторных попыток после коллизии (binary exponential backoff)
    int maxRetries = 10;

    // Базовая задержка отката после коллизии (в секундах)
    // Реальное время ожидания: rand(0, 2^attempt) * backoffSlot
    double backoffSlot = 0.005; // 5 мс

    // Текущее модельное время (в секундах)
    double currentTime = 0.0;

private:
    // Приватный конструктор: снаружи создать экземпляр нельзя
    SimulationParameters() = default;
};
