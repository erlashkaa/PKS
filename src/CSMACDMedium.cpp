#include "CSMACDMedium.h"
#include "Node.h"
#include "Host.h"
#include "SimulationParameters.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>

CSMACDMedium::CSMACDMedium(const std::string& name,
                             double bandwidth,
                             double propagDelay)
    : name_(name),
      bandwidth_(bandwidth),
      propagDelay_(propagDelay),
      isBusy_(false),
      busyUntil_(0.0),
      transmittingCount_(0)
{}

void CSMACDMedium::attachNode(std::shared_ptr<Node> node) {
    nodes_.push_back(node);
}

void CSMACDMedium::subscribe(std::shared_ptr<IObserver> observer) {
    observers_.push_back(observer);
}


void CSMACDMedium::tick(double currentTime) {
    if (isBusy_ && currentTime >= busyUntil_) {
        isBusy_            = false;
        transmittingCount_ = 0;
    }
}


double CSMACDMedium::calcTransmissionTime(const Packet& pkt) const {
    double transmitTime = (pkt.sizeBytes * 8.0) / bandwidth_;
    return transmitTime + propagDelay_;
}

/**
 * sendPacket() — ключевой метод: реализует протокол CSMA/CD.
 *
 * Шаги:
 *   1. Carrier Sense: если канал занят — отказываем (false).
 *   2. Захватываем канал: выставляем isBusy_ и считаем transmittingCount_.
 *   3. Collision Detection: если transmittingCount_ > 1 → коллизия!
 *      - Уведомляем наблюдателей событием COLLISION_DETECTED
 *      - Вычисляем backoff: случайное время ожидания
 *      - Канал освобождается через backoff (busyUntil_ = backoff)
 *      - Возвращаем false, чтобы узел повторил попытку позже
 *   4. Успешная передача:
 *      - Уведомляем о PACKET_SENT
 *      - Доставляем пакет всем узлам на этом сегменте
 *      - Уведомляем о PACKET_DELIVERED
 */
bool CSMACDMedium::sendPacket(const Packet& pkt,
                               const std::string& senderMAC,
                               double currentTime) {

    if (isBusy_) {
        return false; 
    }

    double txTime  = calcTransmissionTime(pkt);
    isBusy_        = true;
    busyUntil_     = currentTime + txTime;
    transmittingCount_++;

    bool collisionOccurred = false;
    if (nodes_.size() > 1) {
        int activeSenders = 0;
        for (const auto& weakNode : nodes_) {
            if (auto node = weakNode.lock()) {
                auto host = std::dynamic_pointer_cast<Host>(node);
                if (host && host->hasTrafficStrategy()) {
                    activeSenders++;
                }
            }
        }
        
        if (activeSenders > 1) {
            int randVal = std::rand() % 100;
            collisionOccurred = (randVal < 15); 
        }
    }

    if (collisionOccurred) {
        auto& params = SimulationParameters::getInstance();
        int maxSlots = 4; 
        int slots    = std::rand() % maxSlots;
        double backoff = slots * params.backoffSlot;

        if (params.enableLogs) {
            std::cout << std::fixed << std::setprecision(3)
                      << "[t=" << currentTime << "] *** КОЛЛИЗИЯ в ["
                      << name_ << "] пакет #" << pkt.id
                      << " | backoff=" << backoff << "с\n";
        }
        busyUntil_ = currentTime + backoff;
        notify(MediumEvent::COLLISION_DETECTED, pkt);
        return false; 
    }

    //  Успешная передача
    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << std::fixed << std::setprecision(3)
                  << "[t=" << currentTime << "] [" << name_
                  << "] передаёт пакет #" << pkt.id
                  << " от " << senderMAC
                  << " [" << pkt.srcIP << " → " << pkt.dstIP << "]"
                  << " (займёт " << txTime * 1000 << " мс)\n";
    }

    notify(MediumEvent::PACKET_SENT, pkt);

    // Доставляем пакет всем узлам в этом сегменте (широковещание на L2)
    for (auto& weakNode : nodes_) {
        if (auto node = weakNode.lock()) {
            if (node->getMAC() != senderMAC) {
                node->receivePacket(pkt);
            }
        }
    }

    notify(MediumEvent::PACKET_DELIVERED, pkt);

    return true;
}

void CSMACDMedium::notify(MediumEvent event, const Packet& pkt) {
    for (auto& obs : observers_) {
        obs->onEvent(event, pkt);
    }
}
