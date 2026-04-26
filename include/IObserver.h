#pragma once
#include "Packet.h"

/**
 * Типы событий, которые генерирует среда передачи (Publisher).
 * StatisticsCollector (Subscriber) реагирует на эти события.
 */
enum class MediumEvent {
    PACKET_SENT,        // Узел начал передачу пакета
    PACKET_DELIVERED,   // Пакет успешно доставлен получателю
    COLLISION_DETECTED  // Произошла коллизия
};

/**
 * IObserver — интерфейс наблюдателя (паттерн Observer / Subscriber).
 *
 * Любой класс, который хочет получать уведомления от CSMACDMedium,
 * должен наследоваться от IObserver и реализовать onEvent().
 *
 * В нашем проекте единственный наблюдатель — StatisticsCollector.
 */
class IObserver {
public:
    virtual ~IObserver() = default;

    /**
     * Вызывается средой передачи (CSMACDMedium) при каждом событии.
     * @param event — тип события (отправка, доставка, коллизия)
     * @param pkt   — пакет, с которым связано событие
     */
    virtual void onEvent(MediumEvent event, const Packet& pkt) = 0;
};
