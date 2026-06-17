#include "Router.h"
#include "CSMACDMedium.h"
#include <iostream>
#include <sstream>
#include <array>
#include "SimulationParameters.h"

Router::Router(const std::string& name,
               const std::string& mac,
               const std::string& ip)
    : Node(name, mac, ip)
{}

void Router::addRoute(const RouteEntry& entry) {
    routingTable_.push_back(entry);
}

void Router::connectInterface(int interfaceId,
                               std::shared_ptr<CSMACDMedium> medium) {
    // Сохраняем интерфейс в карту.
    // Примечание: attachNode() вызывает SimulationFacade::connectRouterInterface(),
    // поэтому здесь мы только сохраняем указатель на среду.
    interfaces_[interfaceId] = medium;
}

/**
 * receivePacket() — логика IP-маршрутизации.
 *
 * Алгоритм:
 *   1. Проверить — пакет для нас? Если да — принять.
 *   2. Найти маршрут в таблице для dstIP пакета.
 *   3. Если маршрут найден — переслать пакет в нужный интерфейс.
 *   4. Если маршрута нет — выбросить пакет (drop) с предупреждением.
 */
void Router::receivePacket(const Packet& pkt) {
    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << "  [Router:" << name_ << "] получил пакет #" << pkt.id
                  << " [" << pkt.srcIP << " → " << pkt.dstIP << "]\n";
    }

    // Если пакет адресован самому маршрутизатору — принять
    if (pkt.dstIP == ip_) {
        if (SimulationParameters::getInstance().enableLogs) {
            std::cout << "  [Router:" << name_ << "] пакет #" << pkt.id
                      << " предназначен мне, принят.\n";
        }
        return;
    }

    // Поиск маршрута в таблице маршрутизации
    const RouteEntry* route = findRoute(pkt.dstIP);

    if (!route) {
        if (SimulationParameters::getInstance().enableLogs) {
            std::cout << "  [Router:" << name_ << "] ⚠ Нет маршрута для "
                      << pkt.dstIP << " — пакет #" << pkt.id << " отброшен!\n";
        }
        return;
    }

    // Найден маршрут — берём нужный интерфейс
    auto it = interfaces_.find(route->interfaceId);
    if (it == interfaces_.end()) {
        if (SimulationParameters::getInstance().enableLogs) {
            std::cout << "  [Router:" << name_ << "] ⚠ Интерфейс "
                      << route->interfaceId << " не подключён!\n";
        }
        return;
    }

    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << "  [Router:" << name_ << "] пересылает пакет #" << pkt.id
                  << " → " << pkt.dstIP
                  << " через интерфейс " << route->interfaceId << "\n";
    }

    // Пересылаем пакет в среду через найденный интерфейс
    it->second->sendPacket(pkt, mac_, SimulationParameters::getInstance().currentTime);
}

/**
 * findRoute() — поиск подходящего маршрута по принципу Longest Prefix Match (LPM).
 * Выбирает маршрут с наиболее специфичной маской (максимальным числом бит).
 */
const RouteEntry* Router::findRoute(const std::string& dstIP) const {
    // Вспомогательная лямбда для подсчета единичных бит в маске
    auto countMaskBits = [](const std::string& mask) -> int {
        std::array<int, 4> octets{};
        std::istringstream ss(mask);
        char dot;
        ss >> octets[0] >> dot >> octets[1] >> dot
           >> octets[2] >> dot >> octets[3];
        int bits = 0;
        for (int o : octets) {
            while (o > 0) {
                if (o & 1) bits++;
                o >>= 1;
            }
        }
        return bits;
    };

    const RouteEntry* bestRoute = nullptr;
    int maxBits = -1;

    for (const auto& entry : routingTable_) {
        if (matchNetwork(dstIP, entry.network, entry.mask)) {
            int bits = countMaskBits(entry.mask);
            if (bits >= maxBits) {
                maxBits = bits;
                bestRoute = &entry;
            }
        }
    }
    return bestRoute;
}

/**
 * matchNetwork() — проверяет, принадлежит ли IP-адрес данной подсети.
 *
 * Алгоритм:
 *   Для каждого из 4 октетов: (ip_octet & mask_octet) == network_octet
 *
 * Пример: IP=192.168.1.5, Net=192.168.1.0, Mask=255.255.255.0
 *   Октет 1: 192 & 255 = 192 == 192 ✓
 *   Октет 2: 168 & 255 = 168 == 168 ✓
 *   Октет 3: 1   & 255 = 1   == 1   ✓
 *   Октет 4: 5   & 0   = 0   == 0   ✓ → совпадение!
 */
bool Router::matchNetwork(const std::string& ip,
                           const std::string& network,
                           const std::string& mask) const {
    // Вспомогательная лямбда: разобрать "a.b.c.d" → {a, b, c, d}
    auto parseIP = [](const std::string& addr) -> std::array<int, 4> {
        std::array<int, 4> octets{};
        std::istringstream ss(addr);
        char dot;
        ss >> octets[0] >> dot >> octets[1] >> dot
           >> octets[2] >> dot >> octets[3];
        return octets;
    };

    auto ipOct  = parseIP(ip);
    auto netOct = parseIP(network);
    auto mskOct = parseIP(mask);

    // Проверяем все 4 октета
    for (int i = 0; i < 4; ++i) {
        if ((ipOct[i] & mskOct[i]) != netOct[i]) return false;
    }
    return true;
}
