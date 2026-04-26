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

    /**
     * Сгенерировать пакет, если пришло время это делать.
     *
     * @param currentTime — текущее время симуляции (в секундах)
     * @param srcMAC      — MAC источника (хоста)
     * @param srcIP       — IP источника
     * @param dstMAC      — MAC назначения
     * @param dstIP       — IP назначения
     * @return            — пакет (если пора отправлять) или пустой optional
     */
    virtual std::optional<Packet> generate(
        double             currentTime,
        const std::string& srcMAC,
        const std::string& srcIP,
        const std::string& dstMAC,
        const std::string& dstIP) = 0;
};
