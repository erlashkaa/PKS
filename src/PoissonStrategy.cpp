#include "PoissonStrategy.h"
#include <sstream>

// =============================================================================
// PoissonStrategy — генерация трафика по пуассоновскому процессу
// =============================================================================

PoissonStrategy::PoissonStrategy(double lambda, uint32_t payloadSize, unsigned seed)
    : lambda_(lambda),
      payloadSize_(payloadSize),
      rng_(seed),
      dist_(lambda),         // Exp(lambda): среднее IAT = 1/lambda
      nextSendTime_(0.0)     // Первый пакет — сразу при старте
{
    // Генерируем первый случайный интервал: когда именно отправить первый пакет
    nextSendTime_ = dist_(rng_);
}

/**
 * generate() для PoissonStrategy:
 *
 * Алгоритм:
 *   1. Если текущее время < nextSendTime_ → ещё не время, вернуть nullopt.
 *   2. Если пришло время → сформировать пакет.
 *   3. Вычислить следующий интервал IAT = Exp(lambda), прибавить к текущему времени.
 *
 * Математическая основа:
 *   Пуассоновский поток с интенсивностью λ пакетов/с означает, что
 *   межпакетные интервалы (IAT) имеют показательное распределение Exp(λ).
 *   Генерируем их через std::exponential_distribution.
 */
std::optional<Packet> PoissonStrategy::generate(
    double             currentTime,
    const std::string& srcMAC,
    const std::string& srcIP,
    const std::string& dstMAC,
    const std::string& dstIP)
{
    // Проверяем: наступило ли запланированное время отправки?
    if (currentTime < nextSendTime_) {
        return std::nullopt;  // Ещё не пора
    }

    // Планируем следующую отправку: следующий случайный IAT ~ Exp(lambda)
    double iat = dist_(rng_);
    nextSendTime_ = currentTime + iat;
    ++packetCount_;

    // Формируем описательный payload с параметрами генератора
    std::ostringstream oss;
    oss << "POISSON[#" << packetCount_
        << "|lam=" << lambda_
        << "|iat=" << iat
        << "]@t=" << currentTime;

    return Packet(srcMAC, dstMAC, srcIP, dstIP, oss.str(), payloadSize_, currentTime);
}
