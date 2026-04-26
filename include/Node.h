#pragma once
#include <string>
#include <memory>
#include "Packet.h"

// Предварительное объявление, чтобы избежать циклических зависимостей
class CSMACDMedium;

/**
 * Node — абстрактный базовый класс для всех сетевых устройств.
 * Хранит общие атрибуты: имя, MAC и IP адрес.
 * Определяет интерфейс receivePacket(), который каждый наследник
 * реализует по-своему (хост принимает данные, роутер — пересылает).
 */
class Node {
public:
    Node(const std::string& name, const std::string& mac, const std::string& ip);
    virtual ~Node() = default;

    // Метод вызывается средой передачи, когда пакет доставлен этому узлу
    virtual void receivePacket(const Packet& pkt) = 0;

    // Привязать узел к среде передачи (канал CSMA/CD)
    void connectTo(std::shared_ptr<CSMACDMedium> medium);

    // Геттеры
    const std::string& getName()    const { return name_; }
    const std::string& getMAC()     const { return mac_;  }
    const std::string& getIP()      const { return ip_;   }

protected:
    std::string name_;  // Человекочитаемое имя устройства (например "Host-A")
    std::string mac_;   // MAC-адрес устройства
    std::string ip_;    // IP-адрес устройства

    // Указатель на среду, к которой подключён узел (может быть nullptr)
    std::shared_ptr<CSMACDMedium> medium_;
};
