#pragma once
#include "Node.h"
#include "ITrafficGenerationStrategy.h"
#include <memory>
#include <string>
#include <queue>

/**
 * Host — конечный узел сети (рабочая станция, сервер и т.д.).
 * Умеет генерировать пакеты согласно выбранной стратегии генерации трафика
 * (паттерн Strategy) и отправлять их через среду CSMA/CD.
 */
class Host : public Node {
public:
    Host(const std::string& name, const std::string& mac, const std::string& ip);

    // Установить стратегию генерации трафика (CBR или Bursty)
    void setTrafficStrategy(std::shared_ptr<ITrafficGenerationStrategy> strategy);

    // Задать целевой узел, которому этот хост будет отправлять пакеты
    void setDestination(const std::string& dstIP, const std::string& dstMAC);

    /**
     * tick() вызывается симулятором на каждом шаге времени (tick).
     * Хост спрашивает у стратегии: «пора ли генерировать пакет?»
     * Если да — ставит пакет в очередь и пытается отправить его в среду.
     */
    void tick(double currentTime);

    // Реализация интерфейса Node: обрабатывает входящий пакет
    void receivePacket(const Packet& pkt) override;

    // Счётчики для хоста
    int getSentCount()     const { return sentCount_;     }
    int getReceivedCount() const { return receivedCount_; }
    int getDroppedCount()  const { return droppedCount_;  }
    size_t getSendQueueSize() const { return sendQueue_.size(); }

    // Проверить наличие стратегии
    bool hasTrafficStrategy() const { return strategy_ != nullptr; }

private:
    std::shared_ptr<ITrafficGenerationStrategy> strategy_; // Стратегия генерации

    std::string dstIP_;   // IP-адрес назначения
    std::string dstMAC_;  // MAC-адрес назначения (в реальности определяется ARP)

    // Очередь пакетов, ожидающих отправки (если канал был занят)
    std::queue<Packet> sendQueue_;

    // Попытаться отправить следующий пакет из очереди в среду
    void trySend(double currentTime);

    int sentCount_     = 0;
    int receivedCount_ = 0;
    int droppedCount_  = 0;
    int currentPacketRetries_ = 0;
};
