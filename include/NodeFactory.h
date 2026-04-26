#pragma once
#include "Node.h"
#include <string>
#include <memory>

/**
 * NodeFactory — фабрика сетевых устройств (паттерн Factory Method).
 *
 * Инкапсулирует логику создания узлов. Клиентский код не знает,
 * какой конкретно подкласс Node создаётся — только передаёт тип.
 *
 * Поддерживаемые типы:
 *   "host"   → объект класса Host
 *   "router" → объект класса Router
 *
 * Расширение: чтобы добавить новый тип устройства (Switch, Bridge...),
 * достаточно добавить ветку в createNode() — без изменения клиентского кода.
 */
class NodeFactory {
public:
    /**
     * Создать сетевой узел заданного типа.
     *
     * @param type — "host" или "router"
     * @param name — имя узла (например "Host-A", "Router-1")
     * @param mac  — MAC-адрес (например "AA:BB:CC:DD:EE:01")
     * @param ip   — IP-адрес  (например "192.168.1.1")
     * @return     — shared_ptr на созданный узел
     * @throws std::invalid_argument если тип неизвестен
     */
    static std::shared_ptr<Node> createNode(
        const std::string& type,
        const std::string& name,
        const std::string& mac,
        const std::string& ip);
};
