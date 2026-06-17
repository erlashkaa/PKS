#include "TrafficStrategies.h"
#include <sstream>

// =============================================================================
// ConstantBitRateStrategy
// =============================================================================

ConstantBitRateStrategy::ConstantBitRateStrategy(double interval,
                                                   uint32_t payloadSize)
    : interval_(interval),
      payloadSize_(payloadSize),
      lastSentTime_(-interval) // Первый пакет отправляется сразу на t=0
{}

/**
 * generate() для CBR:
 *
 * Проверяем, прошло ли достаточно времени с последней отправки.
 * Если да — формируем пакет и обновляем lastSentTime_.
 * Если нет — возвращаем пустой optional (ещё не пора).
 */
std::optional<Packet> ConstantBitRateStrategy::generate(
    double             currentTime,
    const std::string& srcMAC,
    const std::string& srcIP,
    const std::string& dstMAC,
    const std::string& dstIP)
{
    // Проверка: прошёл ли полный интервал с момента последней отправки?
    if (currentTime - lastSentTime_ < interval_) {
        return std::nullopt; // Ещё рано
    }

    lastSentTime_ = currentTime;

    // Формируем payload с меткой времени для наглядности
    std::ostringstream oss;
    oss << "CBR-data@t=" << currentTime;

    return Packet(srcMAC, dstMAC, srcIP, dstIP, oss.str(), payloadSize_, currentTime);
}

// =============================================================================
// BurstyStrategy
// =============================================================================

BurstyStrategy::BurstyStrategy(int      burstSize,
                                 double   burstGap,
                                 double   pauseTime,
                                 uint32_t payloadSize)
    : burstSize_(burstSize),
      burstGap_(burstGap),
      pauseTime_(pauseTime),
      payloadSize_(payloadSize),
      sentInBurst_(0),
      lastEventTime_(-burstGap), // Начинаем отправку сразу
      inPause_(false)
{}

/**
 * generate() для BurstyStrategy:
 *
 * Конечный автомат с двумя состояниями:
 *
 *   [BURST mode]:
 *     - Если прошёл burstGap с последней отправки → отправляем пакет
 *     - Если отправили burstSize пакетов → переходим в PAUSE
 *
 *   [PAUSE mode]:
 *     - Если прошёл pauseTime → возвращаемся в BURST, сбрасываем счётчик
 */
std::optional<Packet> BurstyStrategy::generate(
    double             currentTime,
    const std::string& srcMAC,
    const std::string& srcIP,
    const std::string& dstMAC,
    const std::string& dstIP)
{
    if (inPause_) {
        // Режим паузы между пачками
        if (currentTime - lastEventTime_ >= pauseTime_) {
            // Пауза закончилась — начинаем новую пачку
            inPause_       = false;
            sentInBurst_   = 0;
            lastEventTime_ = currentTime;
        }
        return std::nullopt; // В паузе пакеты не отправляем
    }

    // Режим отправки пачки
    if (currentTime - lastEventTime_ < burstGap_) {
        return std::nullopt; // Ждём следующий интервал внутри пачки
    }

    // Пора отправить пакет в пачке
    lastEventTime_ = currentTime;
    ++sentInBurst_;

    std::ostringstream oss;
    oss << "BURST[" << sentInBurst_ << "/" << burstSize_ << "]@t=" << currentTime;

    Packet pkt(srcMAC, dstMAC, srcIP, dstIP, oss.str(), payloadSize_, currentTime);

    // Проверяем: это был последний пакет в пачке?
    if (sentInBurst_ >= burstSize_) {
        inPause_       = true;
        lastEventTime_ = currentTime; // Начинаем отсчёт паузы
    }

    return pkt;
}
