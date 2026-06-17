#include "NetworkVisualizer.h"
#include "Packet.h"
#include "SimulationConfig.h"
#include <iostream>
#include <iomanip>

// =============================================================================
// NetworkVisualizer — визуализатор событий сети (Observer / Subscriber)
// =============================================================================

NetworkVisualizer::NetworkVisualizer(const std::string& mediumName, bool verbose)
    : mediumName_(mediumName),
      verbose_(verbose)
{}

/**
 * onEvent() — реакция на события от CSMACDMedium (Publisher).
 *
 * Вызывается автоматически через CSMACDMedium::notify() при каждом событии.
 * Выводит цветовые ASCII-метки для наглядной визуализации состояния сети:
 *
 *   ──→  PACKET_SENT:       пакет начал передачу
 *   ✓    PACKET_DELIVERED:  пакет успешно доставлен
 *   ✗✗   COLLISION_DETECTED: коллизия в канале
 */
void NetworkVisualizer::onEvent(MediumEvent event, const Packet& pkt) {
    ++eventsHandled_;

    if (!SimulationConfig::getInstance().enableLogs) {
        return;
    }

    if (!verbose_) {
        // Тихий режим: показываем только коллизии
        if (event != MediumEvent::COLLISION_DETECTED) return;
    }

    // Форматируем вывод с выровненными полями
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "[VIZ:" << mediumName_ << "] ";

    switch (event) {
        case MediumEvent::PACKET_SENT:
            std::cout << "──→  пакет #" << std::setw(4) << pkt.id
                      << "  [" << pkt.srcIP << " → " << pkt.dstIP << "]"
                      << "  ПЕРЕДАЧА\n";
            break;

        case MediumEvent::PACKET_DELIVERED:
            std::cout << " ✓   пакет #" << std::setw(4) << pkt.id
                      << "  [" << pkt.srcIP << " → " << pkt.dstIP << "]"
                      << "  ДОСТАВЛЕН\n";
            break;

        case MediumEvent::COLLISION_DETECTED:
            std::cout << " ✗✗  пакет #" << std::setw(4) << pkt.id
                      << "  [" << pkt.srcIP << " → " << pkt.dstIP << "]"
                      << "  *** КОЛЛИЗИЯ! ***\n";
            break;
    }
}
