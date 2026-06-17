#pragma once
#include <string>
#include <cstdint>

/**
 * Packet — базовая единица данных, передаваемая по сети.
 * Содержит MAC и IP адреса источника/назначения, полезную нагрузку,
 * а также уникальный идентификатор для трассировки.
 */
struct Packet {
    // Уникальный ID пакета (увеличивается с каждым новым пакетом)
    uint32_t id;

    // MAC-адреса (используются на канальном уровне, внутри одного сегмента)
    std::string srcMAC;
    std::string dstMAC;

    // IP-адреса (используются маршрутизаторами для пересылки между сетями)
    std::string srcIP;
    std::string dstIP;

    // Полезная нагрузка (данные)
    std::string payload;

    // Размер пакета в байтах (используется для расчёта времени передачи)
    uint32_t sizeBytes;

    // Время создания пакета (для расчета задержки)
    double creationTime = 0.0;

    // Счётчик глобальных ID (статический, общий для всех пакетов)
    static uint32_t nextId;

    // Конструктор: заполняет поля и автоматически присваивает уникальный ID
    Packet(const std::string& srcMAC, const std::string& dstMAC,
           const std::string& srcIP,  const std::string& dstIP,
           const std::string& payload, uint32_t sizeBytes = 64,
           double creationTime = 0.0)
        : id(nextId++),
          srcMAC(srcMAC), dstMAC(dstMAC),
          srcIP(srcIP),   dstIP(dstIP),
          payload(payload), sizeBytes(sizeBytes),
          creationTime(creationTime)
    {}
};
