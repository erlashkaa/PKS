#pragma once
#include "Node.h"
#include <map>
#include <vector>
#include <string>
#include <memory>

/**
 * Запись в таблице маршрутизации.
 * Определяет, через какой интерфейс (следующий hop) отправить пакет,
 * если его IP-адрес назначения попадает в данную сеть.
 */
struct RouteEntry {
    std::string network;    // Сеть назначения (например "192.168.1.0")
    std::string mask;       // Маска подсети (например "255.255.255.0")
    std::string nextHopIP;  // IP следующего узла (или IP самого интерфейса)
    int         interfaceId;// Номер интерфейса роутера, через который отправить
};

/**
 * Router — маршрутизатор.
 * Получает пакет, ищет подходящий маршрут в таблице маршрутизации
 * и пересылает пакет в нужную среду через соответствующий интерфейс.
 */
class Router : public Node {
public:
    Router(const std::string& name, const std::string& mac, const std::string& ip);

    // Добавить маршрут в таблицу маршрутизации
    void addRoute(const RouteEntry& entry);

    /**
     * Подключить интерфейс маршрутизатора к среде передачи.
     * Роутер может иметь несколько интерфейсов (по одному на сеть).
     * @param interfaceId — номер интерфейса
     * @param medium      — среда, к которой подключён интерфейс
     */
    void connectInterface(int interfaceId, std::shared_ptr<CSMACDMedium> medium);

    // Реализация интерфейса Node: получить пакет и переслать его дальше
    void receivePacket(const Packet& pkt) override;

private:
    // Таблица маршрутизации: список записей
    std::vector<RouteEntry> routingTable_;

    // Карта интерфейсов: interfaceId -> среда передачи
    std::map<int, std::shared_ptr<CSMACDMedium>> interfaces_;

    /**
     * Найти подходящий маршрут для данного IP-адреса назначения.
     * Возвращает указатель на RouteEntry или nullptr, если маршрута нет.
     */
    const RouteEntry* findRoute(const std::string& dstIP) const;

    /**
     * Проверить, принадлежит ли IP-адрес данной сети.
     * Упрощённая реализация: побайтовое сравнение с маской.
     */
    bool matchNetwork(const std::string& ip,
                      const std::string& network,
                      const std::string& mask) const;
};
