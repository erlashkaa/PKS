#pragma once
#include "ITrafficGenerationStrategy.h"
#include <random>

/**
 * PoissonStrategy — стратегия генерации трафика по пуассоновскому процессу
 * (паттерн Strategy, реализация #3).
 *
 * Межпакетные интервалы (IAT) распределены по показательному закону:
 *   IAT ~ Exp(lambda), где lambda = средняя интенсивность (пакетов/секунду)
 *
 * Это стандартная модель «случайного» сетевого трафика в телекоммуникациях.
 * При lambda=2.0 → в среднем 2 пакета в секунду, но интервалы случайны.
 *
 * Временна́я диаграмма (пример при lambda=2):
 *   0.0s → пакет, 0.8s → пакет, 1.1s → пакет, 1.9s → пакет ...
 *             ^нерегулярные интервалы, но средняя частота = 2/с
 */
class PoissonStrategy : public ITrafficGenerationStrategy {
public:
    /**
     * @param lambda      — средняя интенсивность генерации пакетов (пакетов/секунду)
     * @param payloadSize — размер полезной нагрузки пакета (байты)
     * @param seed        — начальное значение ГПСЧ (0 = случайное)
     */
    explicit PoissonStrategy(double   lambda      = 2.0,
                             uint32_t payloadSize = 64,
                             unsigned seed        = 42);

    std::optional<Packet> generate(
        double             currentTime,
        const std::string& srcMAC,
        const std::string& srcIP,
        const std::string& dstMAC,
        const std::string& dstIP) override;

private:
    double   lambda_;         // Интенсивность (пакетов/с)
    uint32_t payloadSize_;    // Размер пакета (байты)

    std::mt19937                          rng_;   // Вихрь Мерсенна
    std::exponential_distribution<double> dist_;  // Exp(lambda)

    double nextSendTime_;     // Момент времени следующей отправки
    int    packetCount_ = 0;  // Счётчик отправленных пакетов (для payload-метки)
};
