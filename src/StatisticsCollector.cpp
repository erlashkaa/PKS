#include "StatisticsCollector.h"
#include "SimulationParameters.h"
#include <iostream>
#include <iomanip>

/**
 * onEvent() — реакция на события от среды передачи (CSMACDMedium).
 *
 * Этот метод вызывается автоматически каждый раз, когда CSMACDMedium
 * вызывает notify(). Мы просто инкрементируем нужный счётчик.
 */
void StatisticsCollector::onEvent(MediumEvent event, const Packet& pkt) {
    switch (event) {
        case MediumEvent::PACKET_SENT:
            ++packetsSent_;
            break;
        case MediumEvent::PACKET_DELIVERED:
            ++packetsDelivered_;
            {
                double delay = SimulationParameters::getInstance().currentTime - pkt.creationTime;
                if (delay > 0.0) {
                    totalDelay_ += delay;
                }
            }
            break;
        case MediumEvent::COLLISION_DETECTED:
            ++collisions_;
            break;
    }
}

/**
 * printReport() — выводит итоговую таблицу статистики в stdout.
 */
void StatisticsCollector::printReport() const {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║       СТАТИСТИКА СИМУЛЯЦИИ           ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║  Отправлено пакетов  : "
              << std::setw(13) << packetsSent_      << " ║\n";
    std::cout << "║  Доставлено пакетов  : "
              << std::setw(13) << packetsDelivered_ << " ║\n";
    std::cout << "║  Коллизий            : "
              << std::setw(13) << collisions_       << " ║\n";

    // Процент успешной доставки (delivery rate)
    double rate = (packetsSent_ > 0)
        ? (100.0 * packetsDelivered_ / packetsSent_)
        : 0.0;
    std::cout << "║  Успешность доставки : "
              << std::setw(11) << std::fixed << std::setprecision(1)
              << rate << "%" << " ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
}

void StatisticsCollector::reset() {
    packetsSent_      = 0;
    packetsDelivered_ = 0;
    collisions_       = 0;
    packetsDropped_   = 0;
    totalDelay_       = 0.0;
}
