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

/**
 * tick() — вызывается на каждом шаге симуляции.
 *
 * Логика:
 *   1. Если задана стратегия — спросить у неё, нужно ли генерировать пакет.
 *   2. Если пакет сгенерирован — положить его в очередь отправки.
 *   3. Попытаться отправить первый пакет из очереди.
 */
void Host::tick(double currentTime) {
    // Шаг 1-2: Генерация пакета по стратегии
    if (strategy_ && !dstIP_.empty()) {
        auto maybePkt = strategy_->generate(
            currentTime, mac_, ip_, dstMAC_, dstIP_);
        if (maybePkt.has_value()) {
            sendQueue_.push(maybePkt.value());
        }
    }

    // Шаг 3: Попытаться отправить
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

    // CSMA: Carrier Sense — проверяем, свободен ли канал
    if (!medium_->isIdle()) {
        // Канал занят — ждём, не делаем ничего в этом тике
        return;
    }

    // Канал свободен — отправляем пакет
    const Packet& pkt = sendQueue_.front();

    std::cout << std::fixed << std::setprecision(3)
              << "[t=" << currentTime << "] "
              << name_ << " → пытается отправить пакет #" << pkt.id
              << " [" << pkt.srcIP << " → " << pkt.dstIP << "] "
              << "через " << medium_->getName() << "\n";

    bool accepted = medium_->sendPacket(pkt, mac_, currentTime);

    if (accepted) {
        // Пакет принят средой — убираем из очереди
        sendQueue_.pop();
        ++sentCount_;
    }
    // Если не принят (коллизия внутри sendPacket) — оставляем в очереди,
    // CSMACDMedium сам разблокирует канал через backoff
}

/**
 * receivePacket() — обрабатывает входящий пакет.
 * Хост проверяет, что пакет адресован ему (по IP),
 * и выводит сообщение о получении.
 */
void Host::receivePacket(const Packet& pkt) {
    if (pkt.dstIP != ip_ && pkt.dstMAC != mac_) {
        // Пакет не для нас (широковещательная среда — слышат все)
        return;
    }
    ++receivedCount_;
    std::cout << std::fixed << std::setprecision(3)
              << "[t=" << SimulationParameters::getInstance().currentTime << "] "
              << name_ << " ✓ получил пакет #" << pkt.id
              << " от " << pkt.srcIP
              << " | payload: \"" << pkt.payload << "\"\n";
}
