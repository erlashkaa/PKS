#pragma once
#include "ITrafficGenerationStrategy.h"

/**
 * ConstantBitRateStrategy — стратегия CBR (паттерн Strategy, реализация #1).
 *
 * Генерирует пакеты через строго равные промежутки времени.
 * Например: каждые 0.5 секунды — один пакет.
 */
class ConstantBitRateStrategy : public ITrafficGenerationStrategy {
public:
    /**
     * @param interval    — интервал между пакетами (в секундах)
     * @param payloadSize — размер полезной нагрузки (байты)
     */
    explicit ConstantBitRateStrategy(double interval = 0.5, uint32_t payloadSize = 64);

    std::optional<Packet> generate(
        double             currentTime,
        const std::string& srcMAC,
        const std::string& srcIP,
        const std::string& dstMAC,
        const std::string& dstIP) override;

private:
    double   interval_;     // Интервал между пакетами (с)
    uint32_t payloadSize_;  // Размер пакета (байты)
    double   lastSentTime_; // Время отправки предыдущего пакета
};

// =============================================================================

/**
 * BurstyStrategy — стратегия «пачечной» отправки (паттерн Strategy, реализация #2).
 *
 * Отправляет N пакетов подряд (burst), затем молчит паузу (pause),
 * после чего снова отправляет burst и т.д.
 *
 * Временна́я диаграмма:
 *   [пакет][пакет][пакет] --- пауза --- [пакет][пакет][пакет] --- ...
 */
class BurstyStrategy : public ITrafficGenerationStrategy {
public:
    /**
     * @param burstSize   — количество пакетов в одной пачке
     * @param burstGap    — интервал между пакетами ВНУТРИ пачки (с)
     * @param pauseTime   — пауза между пачками (с)
     * @param payloadSize — размер пакета (байты)
     */
    BurstyStrategy(int burstSize  = 3,
                   double burstGap   = 0.05,
                   double pauseTime  = 1.0,
                   uint32_t payloadSize = 64);

    std::optional<Packet> generate(
        double             currentTime,
        const std::string& srcMAC,
        const std::string& srcIP,
        const std::string& dstMAC,
        const std::string& dstIP) override;

private:
    int      burstSize_;      // Пакетов в одной пачке
    double   burstGap_;       // Пауза между пакетами в пачке (с)
    double   pauseTime_;      // Пауза между пачками (с)
    uint32_t payloadSize_;    // Размер пакета (байты)

    int    sentInBurst_;      // Сколько пакетов уже отправлено в текущей пачке
    double lastEventTime_;    // Время последнего события (отправки или начала паузы)
    bool   inPause_;          // Сейчас пауза между пачками?
};
