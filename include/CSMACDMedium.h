#pragma once
#include "Packet.h"
#include "IObserver.h"
#include <vector>
#include <memory>
#include <string>

// Предварительное объявление
class Node;


/**
 * CSMACDMedium — общая среда передачи данных с поддержкой протокола CSMA/CD.
 *
 * CSMA/CD (Carrier Sense Multiple Access with Collision Detection):
 *  - Carrier Sense   : перед отправкой узел «слушает» канал
 *  - Multiple Access : несколько узлов делят одну среду
 *  - Collision Det.  : если два узла отправили одновременно — коллизия
 *
 * Паттерн Observer: CSMACDMedium — Publisher.
 * При каждом событии (отправка, доставка, коллизия) уведомляет всех подписчиков.
 */
class CSMACDMedium {
public:
    /**
     * @param name        — имя среды (для логирования)
     * @param bandwidth   — пропускная способность в бит/с
     * @param propagDelay — задержка распространения сигнала в секундах
     */
    CSMACDMedium(const std::string& name,
                 double bandwidth   = 10e6,   // 10 Мбит/с
                 double propagDelay = 0.001); // 1 мс

    // Подключить узел к этой среде
    void attachNode(std::shared_ptr<Node> node);

    /**
     * Попытаться отправить пакет от узла sender.
     * Реализует логику CSMA/CD:
     *   1. Проверить, свободен ли канал (carrierSense)
     *   2. Если занят — вернуть false (узел должен подождать)
     *   3. Если свободен — начать передачу
     *   4. Проверить коллизию (если другой узел тоже начал передачу)
     *   5. При коллизии — уведомить наблюдателей и завершить передачу с ошибкой
     *   6. При успехе — доставить пакет всем узлам и уведомить наблюдателей
     * @return true, если пакет был принят средой для передачи
     */
    bool sendPacket(const Packet& pkt, const std::string& senderMAC, double currentTime);

    // Обновить состояние среды (освободить канал, если передача завершена)
    void tick(double currentTime);

    // Проверить, свободен ли канал прямо сейчас
    bool isIdle() const { return !isBusy_; }

    // Рассчитать время передачи пакета (в секундах)
    double calcTransmissionTime(const Packet& pkt) const;

    // ---- Observer API ----
    // Подписать наблюдателя на события среды
    void subscribe(std::shared_ptr<IObserver> observer);

    const std::string& getName() const { return name_; }

private:
    std::string name_;
    double bandwidth_;    // Пропускная способность (бит/с)
    double propagDelay_;  // Задержка распространения (с)

    bool   isBusy_      = false; // Канал занят?
    double busyUntil_   = 0.0;   // До какого момента времени занят

    // Количество узлов, одновременно начавших передачу (для обнаружения коллизий)
    int    transmittingCount_ = 0;

    // Узлы, подключённые к этой среде
    std::vector<std::weak_ptr<Node>> nodes_;

    // Список подписчиков-наблюдателей (Observer pattern)
    std::vector<std::shared_ptr<IObserver>> observers_;

    // Уведомить всех подписчиков о событии
    void notify(MediumEvent event, const Packet& pkt);
};
