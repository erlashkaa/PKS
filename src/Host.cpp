#include "Host.h"
#include "CSMACDMedium.h"
#include "SimulationParameters.h"
#include <iostream>
#include <iomanip>

Host::Host(const std::string& name,
           const std::string& mac,
           const std::string& ip)
    : Node(name, mac, ip)
{}

void Host::setTrafficStrategy(std::shared_ptr<ITrafficGenerationStrategy> strategy) {
    strategy_ = strategy;
}

void Host::setDestination(const std::string& dstIP, const std::string& dstMAC) {
    dstIP_  = dstIP;
    dstMAC_ = dstMAC;
}


void Host::tick(double currentTime) {
    if (strategy_ && !dstIP_.empty()) {
        auto maybePkt = strategy_->generate(
            currentTime, mac_, ip_, dstMAC_, dstIP_);
        if (maybePkt.has_value()) {
            sendQueue_.push(maybePkt.value());
        }
    }
    trySend(currentTime);
}

/**
 * trySend() — пытается отправить первый пакет из очереди.
 *
 * CSMA: сначала "слушаем" канал (carrierSense через isIdle()).
 * Если канал свободен — отправляем. Если занят — ждём следующего тика.
 *
 * Повторные попытки при коллизии управляются через retryTime_
 * (задержка Binary Exponential Backoff задаётся в CSMACDMedium).
 */
void Host::trySend(double currentTime) {
    if (sendQueue_.empty() || !medium_) return;
    if (!medium_->isIdle()) {
        return;
    }

    const Packet& pkt = sendQueue_.front();

    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << std::fixed << std::setprecision(3)
                  << "[t=" << currentTime << "] "
                  << name_ << " → пытается отправить пакет #" << pkt.id
                  << " [" << pkt.srcIP << " → " << pkt.dstIP << "] "
                  << "через " << medium_->getName() << "\n";
    }

    bool accepted = medium_->sendPacket(pkt, mac_, currentTime);

    if (accepted) {
        sendQueue_.pop();
        ++sentCount_;
        currentPacketRetries_ = 0;
    } else {
        currentPacketRetries_++;
        if (currentPacketRetries_ >= SimulationParameters::getInstance().maxRetries) {
            if (SimulationParameters::getInstance().enableLogs) {
                std::cout << std::fixed << std::setprecision(3)
                          << "[t=" << currentTime << "] " << name_
                          << " ⚠ Пакет #" << pkt.id << " отброшен после "
                          << currentPacketRetries_ << " коллизий!\n";
            }
            sendQueue_.pop();        
            ++droppedCount_;           
            currentPacketRetries_ = 0; 
        }
    }
}

void Host::receivePacket(const Packet& pkt) {
    if (pkt.dstIP != ip_ && pkt.dstMAC != mac_) {
        return;
    }
    ++receivedCount_;
    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << std::fixed << std::setprecision(3)
                  << "[t=" << SimulationParameters::getInstance().currentTime << "] "
                  << name_ << " ✓ получил пакет #" << pkt.id
                  << " от " << pkt.srcIP
                  << " | payload: \"" << pkt.payload << "\"\n";
    }
}
