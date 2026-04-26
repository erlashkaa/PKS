#include "Node.h"
#include "CSMACDMedium.h"

// Конструктор: инициализируем имя, MAC и IP узла
Node::Node(const std::string& name,
           const std::string& mac,
           const std::string& ip)
    : name_(name), mac_(mac), ip_(ip), medium_(nullptr)
{}

// Подключить узел к среде передачи.
// После этого узел может вызывать medium_->sendPacket() для отправки.
void Node::connectTo(std::shared_ptr<CSMACDMedium> medium) {
    medium_ = medium;
}
