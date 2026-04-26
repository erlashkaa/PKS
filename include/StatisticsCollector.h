#pragma once
#include "IObserver.h"

/**
 * StatisticsCollector — сборщик статистики (паттерн Observer / Subscriber).
 *
 * Подписывается на события CSMACDMedium и ведёт счётчики:
 *   - packetsSent      : сколько пакетов было отправлено
 *   - packetsDelivered : сколько пакетов успешно доставлено
 *   - collisions       : сколько коллизий зафиксировано
 *
 * При желании можно подключить один экземпляр ко многим средам,
 * и он будет собирать агрегированную статистику по всей сети.
 */
class StatisticsCollector : public IObserver {
public:
    StatisticsCollector() = default;

    // Реализация интерфейса IObserver
    void onEvent(MediumEvent event, const Packet& pkt) override;

    // ---- Геттеры ----
    int getPacketsSent()      const { return packetsSent_;      }
    int getPacketsDelivered() const { return packetsDelivered_; }
    int getCollisions()       const { return collisions_;       }

    // Вывести итоговую статистику в stdout
    void printReport() const;

    // Сбросить все счётчики
    void reset();

private:
    int packetsSent_      = 0;
    int packetsDelivered_ = 0;
    int collisions_       = 0;
};
