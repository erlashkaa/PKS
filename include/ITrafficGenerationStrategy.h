#pragma once
#include "Packet.h"
#include <string>
#include <optional>

/**
 * ITrafficGenerationStrategy — интерфейс стратегии генерации трафика
 * (паттерн Strategy).
 *
 * Позволяет менять алгоритм генерации пакетов в Host без изменения
 * самого класса Host. Достаточно передать другой объект стратегии.
 *
 * Конкретные реализации:
 *   - ConstantBitRateStrategy : пакеты через равные промежутки времени
 *   - BurstyStrategy          : пакеты пачками с паузами между пачками
 */
class ITrafficGenerationStrategy {
public:
    virtual ~ITrafficGenerationStrategy() = default;

    virtual std::optional<Packet> generate(
        double             currentTime,
        const std::string& srcMAC,
        const std::string& srcIP,
        const std::string& dstMAC,
        const std::string& dstIP) = 0;
};
