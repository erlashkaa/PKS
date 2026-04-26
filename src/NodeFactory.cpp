#include "NodeFactory.h"
#include "Host.h"
#include "Router.h"
#include <stdexcept>
#include <algorithm>

/**
 * createNode() — фабричный метод (паттерн Factory Method).
 *
 * По строке типа создаёт нужный подкласс Node.
 * Клиентский код работает только с базовым интерфейсом Node*.
 *
 * Расширение: добавить новый тип устройства легко —
 * нужно лишь добавить ветку else-if здесь.
 */
std::shared_ptr<Node> NodeFactory::createNode(
    const std::string& type,
    const std::string& name,
    const std::string& mac,
    const std::string& ip)
{
    // Приводим тип к нижнему регистру для удобства (case-insensitive)
    std::string ltype = type;
    std::transform(ltype.begin(), ltype.end(), ltype.begin(), ::tolower);

    if (ltype == "host") {
        return std::make_shared<Host>(name, mac, ip);
    }
    else if (ltype == "router") {
        return std::make_shared<Router>(name, mac, ip);
    }
    else {
        throw std::invalid_argument(
            "NodeFactory: неизвестный тип узла '" + type + "'."
            " Допустимые значения: 'host', 'router'");
    }
}
