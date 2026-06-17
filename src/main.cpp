/**
 * main.cpp — точка входа симулятора компьютерной сети.
 *
 * Демонстрирует ВСЕ четыре паттерна проектирования:
 *
 *   1. Singleton  : SimulationConfig::getInstance() — единая точка конфигурации
 *   2. Factory    : NodeFactory::createNode() — создание узлов через фабрику
 *   3. Strategy   : ConstantBitRateStrategy, PoissonStrategy, BurstyStrategy
 *   4. Observer   : CSMACDMedium → StatisticsCollector + NetworkVisualizer
 *
 * Топология сети:
 *
 *   Host-A (CBR,  192.168.1.2) ──┐
 *   Host-C (Pois, 192.168.1.3) ──┤── [LAN1] ── Router-1 ── [LAN2] ── Host-B (Bursty, 192.168.2.2)
 *
 * Симуляция запускается на 5 секунд через SimulationFacade.
 */

#include "SimulationFacade.h"
#include "TrafficStrategies.h"
#include "PoissonStrategy.h"
#include "SimulationConfig.h"
#include "NetworkVisualizer.h"
#include "Router.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#endif

// Настроить UTF-8 консоль Windows
void setupUtf8Console() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void printUsage(const char* programName) {
    std::cout << "Использование: " << programName << " [опции]\n\n"
              << "Опции:\n"
              << "  -d, --duration <сек>   Длительность симуляции в секундах (по умолчанию: 5)\n"
              << "  -h, --help             Показать эту справку\n";
}

double parseDuration(int argc, char* argv[]) {
    double duration = 5.0;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            std::exit(0);
        }
        if ((std::strcmp(argv[i], "-d") == 0 || std::strcmp(argv[i], "--duration") == 0) && i + 1 < argc) {
            duration = std::atof(argv[++i]);
            if (duration <= 0.0) {
                std::cerr << "Ошибка: длительность должна быть положительным числом.\n";
                std::exit(1);
            }
        }
    }

    return duration;
}

int main(int argc, char* argv[]) {
    // Инициализируем ГПСЧ (для CSMA/CD backoff)
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    setupUtf8Console();

    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║   СИМУЛЯТОР КОМПЬЮТЕРНОЙ СЕТИ (CSMA/CD)      ║\n";
    std::cout << "║   Паттерны: Singleton | Factory | Strategy   ║\n";
    std::cout << "║             Observer                         ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";

    // =========================================================================
    // 1. SINGLETON — SimulationConfig
    //    Единственный экземпляр, доступный из любой точки программы.
    // =========================================================================
    auto& cfg = SimulationConfig::getInstance();
    cfg.tickStep       = 0.001;  // Тик = 1 мс
    cfg.backoffSlot    = 0.005;  // Слот backoff = 5 мс
    cfg.maxRetries     = 10;
    cfg.speedMultiplier = 100.0;
    cfg.enableLogs     = true;

    std::cout << "[Singleton: SimulationConfig]\n";
    std::cout << "  tickStep      = " << cfg.tickStep * 1000    << " мс\n";
    std::cout << "  backoffSlot   = " << cfg.backoffSlot * 1000 << " мс\n";
    std::cout << "  maxRetries    = " << cfg.maxRetries          << "\n\n";

    // =========================================================================
    // 2. FACADE — SimulationFacade скрывает детали построения топологии.
    //    Внутри addHost/addRouter используется NodeFactory (Factory Method).
    // =========================================================================
    SimulationFacade sim;

    // ---- Создаём узлы (NodeFactory внутри Facade) ----
    std::cout << "[Factory Method: NodeFactory]\n";
    auto hostA  = sim.addHost("Host-A", "AA:BB:CC:DD:EE:01", "192.168.1.2");
    auto hostB  = sim.addHost("Host-B", "AA:BB:CC:DD:EE:02", "192.168.2.2");
    auto hostC  = sim.addHost("Host-C", "AA:BB:CC:DD:EE:03", "192.168.1.3");
    auto router = sim.addRouter("Router-1", "AA:BB:CC:DD:EE:FF", "192.168.1.1");
    std::cout << "\n";

    // ---- Создаём среды передачи ----
    auto lan1 = sim.addMedium("LAN1", 10e6, 0.001); // 10 Мбит/с, delay=1мс
    auto lan2 = sim.addMedium("LAN2", 10e6, 0.002); // 10 Мбит/с, delay=2мс
    std::cout << "\n";

    // =========================================================================
    // 3. OBSERVER — подписываем два наблюдателя на каждую среду:
    //      • StatisticsCollector — агрегирует счётчики
    //      • NetworkVisualizer   — отображает события в консоли
    //    StatisticsCollector подключается внутри sim.addMedium() автоматически.
    //    NetworkVisualizer подключаем явно, чтобы продемонстрировать API.
    // =========================================================================
    std::cout << "[Observer: регистрация подписчиков]\n";
    auto vizLan1 = std::make_shared<NetworkVisualizer>("LAN1", false); // тихий режим
    auto vizLan2 = std::make_shared<NetworkVisualizer>("LAN2", true);  // подробный
    lan1->subscribe(vizLan1);
    lan2->subscribe(vizLan2);
    std::cout << "  NetworkVisualizer подписан на LAN1 (коллизии) и LAN2 (всё)\n\n";

    // ---- Подключаем узлы к средам ----
    sim.connectNodeToMedium(hostA, lan1);           // Host-A ↔ LAN1
    sim.connectNodeToMedium(hostC, lan1);           // Host-C ↔ LAN1
    sim.connectRouterInterface(router, 0, lan1);    // Router iface-0 ↔ LAN1
    sim.connectRouterInterface(router, 1, lan2);    // Router iface-1 ↔ LAN2
    sim.connectNodeToMedium(hostB, lan2);           // Host-B ↔ LAN2
    std::cout << "\n";

    // ---- Таблица маршрутизации ----
    router->addRoute({"192.168.1.0", "255.255.255.0", "192.168.1.2", 0});
    router->addRoute({"192.168.2.0", "255.255.255.0", "192.168.2.2", 1});
    std::cout << "[Маршрутизация] Router-1:\n";
    std::cout << "  192.168.1.0/24 → iface-0 (LAN1)\n";
    std::cout << "  192.168.2.0/24 → iface-1 (LAN2)\n\n";

    // =========================================================================
    // 4. STRATEGY — назначаем разные алгоритмы генерации трафика:
    //      • Host-A : ConstantBitRateStrategy (CBR) — пакет каждые 0.5с
    //      • Host-B : BurstyStrategy           — 3 пакета пачкой, пауза 2с
    //      • Host-C : PoissonStrategy          — случайный трафик, λ=3 пак/с
    // =========================================================================
    std::cout << "[Strategy: назначение стратегий генерации трафика]\n";

    // Host-A → ConstantBitRateStrategy
    auto cbrStrategy = std::make_shared<ConstantBitRateStrategy>(
        0.5,   // интервал 500 мс между пакетами
        128    // размер пакета 128 байт
    );
    hostA->setTrafficStrategy(cbrStrategy);
    hostA->setDestination("192.168.2.2", "AA:BB:CC:DD:EE:FF");
    std::cout << "  Host-A : ConstantBitRate  (interval=500мс, size=128B)\n";

    // Host-B → BurstyStrategy
    auto burstyStrategy = std::make_shared<BurstyStrategy>(
        3,     // 3 пакета в пачке
        0.05,  // 50 мс между пакетами внутри пачки
        2.0,   // 2 секунды паузы между пачками
        64     // размер пакета 64 байта
    );
    hostB->setTrafficStrategy(burstyStrategy);
    hostB->setDestination("192.168.1.2", "AA:BB:CC:DD:EE:FF");
    std::cout << "  Host-B : Bursty           (burst=3, gap=50мс, pause=2с, size=64B)\n";

    // Host-C → PoissonStrategy (НОВАЯ стратегия)
    auto poissonStrategy = std::make_shared<PoissonStrategy>(
        3.0,   // λ=3 пакетов в секунду (среднее)
        96,    // размер пакета 96 байт
        42     // seed для воспроизводимости
    );
    hostC->setTrafficStrategy(poissonStrategy);
    hostC->setDestination("192.168.2.2", "AA:BB:CC:DD:EE:FF");
    std::cout << "  Host-C : Poisson          (lambda=3.0 пак/с, size=96B)\n\n";

    // =========================================================================
    // 5. Запуск симуляции (длительность задаётся аргументом командной строки)
    // =========================================================================
    const double duration = parseDuration(argc, argv);
    sim.startSimulation(duration);

    // =========================================================================
    // 6. Итоговая статистика
    // =========================================================================
    sim.printStats();
    std::cout << "\n[Observer: NetworkVisualizer обработал "
              << vizLan1->getEventsHandled() + vizLan2->getEventsHandled()
              << " событий суммарно]\n";

    std::cout << "\nПрограмма завершена.\n";
    return 0;
}
