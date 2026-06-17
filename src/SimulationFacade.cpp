#include "SimulationFacade.h"
#include "NodeFactory.h"
#include "SimulationParameters.h"
#include <iostream>

SimulationFacade::SimulationFacade() {
    // Создаём единственный сборщик статистики
    stats_ = std::make_shared<StatisticsCollector>();
}

// ---- Построение топологии ----

std::shared_ptr<Host> SimulationFacade::addHost(
    const std::string& name,
    const std::string& mac,
    const std::string& ip)
{
    // Используем фабрику (Factory Method) для создания узла
    auto node = NodeFactory::createNode("host", name, mac, ip);
    auto host = std::dynamic_pointer_cast<Host>(node);
    hosts_.push_back(host);

    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << "[Топология] Добавлен Host: " << name
                  << " | MAC=" << mac << " | IP=" << ip << "\n";
    }
    return host;
}

std::shared_ptr<Router> SimulationFacade::addRouter(
    const std::string& name,
    const std::string& mac,
    const std::string& ip)
{
    auto node   = NodeFactory::createNode("router", name, mac, ip);
    auto router = std::dynamic_pointer_cast<Router>(node);
    routers_.push_back(router);

    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << "[Топология] Добавлен Router: " << name
                  << " | MAC=" << mac << " | IP=" << ip << "\n";
    }
    return router;
}

std::shared_ptr<CSMACDMedium> SimulationFacade::addMedium(
    const std::string& name,
    double bandwidth,
    double propagDelay)
{
    auto medium = std::make_shared<CSMACDMedium>(name, bandwidth, propagDelay);

    // Подписываем сборщик статистики на события этой среды
    medium->subscribe(stats_);

    media_.push_back(medium);

    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << "[Топология] Добавлена среда: " << name
                  << " | BW=" << bandwidth / 1e6 << " Мбит/с"
                  << " | delay=" << propagDelay * 1000 << " мс\n";
    }
    return medium;
}

void SimulationFacade::connectNodeToMedium(
    std::shared_ptr<Node>         node,
    std::shared_ptr<CSMACDMedium> medium)
{
    // Подключаем узел к среде в обоих направлениях:
    //  - узел знает о среде (через connectTo) → может отправлять
    //  - среда знает об узле (через attachNode) → может доставлять
    node->connectTo(medium);
    medium->attachNode(node);

    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << "[Топология] " << node->getName()
                  << " подключён к [" << medium->getName() << "]\n";
    }
}

void SimulationFacade::connectRouterInterface(
    std::shared_ptr<Router>       router,
    int                           interfaceId,
    std::shared_ptr<CSMACDMedium> medium)
{
    // Для маршрутизатора: подключаем конкретный интерфейс к среде
    router->connectInterface(interfaceId, medium);
    medium->attachNode(router);

    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << "[Топология] " << router->getName()
                  << " интерфейс-" << interfaceId
                  << " подключён к [" << medium->getName() << "]\n";
    }
}

// ---- Управление симуляцией ----

/**
 * startSimulation() — главный цикл симуляции (паттерн Facade).
 *
 * Скрывает от пользователя детали реализации:
 *   1. Настраивает параметры из Singleton (SimulationParameters)
 *   2. Запускает цикл тиков
 *   3. На каждом тике:
 *      a. Обновляет все среды (освобождает каналы)
 *      b. Вызывает tick() у всех хостов (генерация трафика + отправка)
 *   4. Выводит разделители для удобства чтения лога
 */
void SimulationFacade::startSimulation(double durationSeconds) {
    auto& params = SimulationParameters::getInstance();

    running_     = true;
    currentTime_ = 0.0;

    double tickStep = params.tickStep;
    // Общее количество тиков
    long long totalTicks = static_cast<long long>(durationSeconds / tickStep);

    if (params.enableLogs) {
        std::cout << "\n";
        std::cout << "════════════════════════════════════════════\n";
        std::cout << "  СИМУЛЯЦИЯ ЗАПУЩЕНА\n";
        std::cout << "  Длительность : " << durationSeconds << " с\n";
        std::cout << "  Шаг тика     : " << tickStep * 1000 << " мс\n";
        std::cout << "  Всего тиков  : " << totalTicks << "\n";
        std::cout << "════════════════════════════════════════════\n\n";
    }

    // Главный цикл симуляции
    for (long long tick = 0; tick < totalTicks && running_; ++tick) {
        currentTime_ = tick * tickStep;
        params.currentTime = currentTime_;

        // Шаг a: Обновляем все среды передачи
        for (auto& medium : media_) {
            medium->tick(currentTime_);
        }

        // Шаг b: Все хосты действуют (генерируют трафик и пытаются отправить)
        for (auto& host : hosts_) {
            host->tick(currentTime_);
        }
    }

    running_ = false;

    if (params.enableLogs) {
        std::cout << "\n════════════════════════════════════════════\n";
        std::cout << "  СИМУЛЯЦИЯ ЗАВЕРШЕНА (t=" << durationSeconds << "с)\n";
        std::cout << "════════════════════════════════════════════\n";
    }
}

void SimulationFacade::stopSimulation() {
    running_ = false;
    if (SimulationParameters::getInstance().enableLogs) {
        std::cout << "[Facade] Симуляция остановлена досрочно.\n";
    }
}

void SimulationFacade::printStats() const {
    if (SimulationParameters::getInstance().enableLogs) {
        // Выводим статистику из хостов
        std::cout << "\n--- Статистика хостов ---\n";
        for (const auto& host : hosts_) {
            std::cout << "  " << host->getName()
                      << " | Отправлено: " << host->getSentCount()
                      << " | Получено: "   << host->getReceivedCount() << "\n";
        }

        // Выводим агрегированную статистику среды (Observer)
        stats_->printReport();
    }
}
