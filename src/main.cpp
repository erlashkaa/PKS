/**
 * main.cpp — точка входа симулятора компьютерной сети.
 *
 * Собирает следующую топологию:
 *
 *   Host-A (192.168.1.2) ──[Medium-LAN1]── Router ──[Medium-LAN2]── Host-B (192.168.2.2)
 *
 * Host-A → использует стратегию ConstantBitRate (пакет каждые 0.5с)
 * Host-B → использует стратегию Bursty (3 пакета пачкой, затем пауза 2с)
 *
 * Симуляция запускается на 10 секунд через SimulationFacade.
 * По завершении выводится таблица статистики.
 */

#include "SimulationFacade.h"
#include "TrafficStrategies.h"
#include "SimulationParameters.h"
#include "Router.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

// Set UTF-8 console mode for Windows
void setupUtf8Console() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

int main() {
    // Инициализируем генератор случайных чисел (для коллизий и backoff)
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Настроить UTF-8 для корректного вывода русских символов
    setupUtf8Console();

    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║   СИМУЛЯТОР КОМПЬЮТЕРНОЙ СЕТИ (CSMA/CD)      ║\n";
    std::cout << "║   Учебный проект | Паттерны ООП              ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";

    // =========================================================================
    // 1. Настройка глобальных параметров симуляции (Singleton)
    // =========================================================================
    auto& params = SimulationParameters::getInstance();
    params.tickStep        = 0.001; // Тик = 1 мс
    params.backoffSlot     = 0.005; // Слот backoff = 5 мс
    params.maxRetries      = 10;

    std::cout << "[Конфигурация] tickStep=" << params.tickStep * 1000
              << "мс | backoffSlot=" << params.backoffSlot * 1000
              << "мс | maxRetries=" << params.maxRetries << "\n\n";

    // =========================================================================
    // 2. Создание топологии через SimulationFacade (Facade pattern)
    // =========================================================================
    SimulationFacade sim;

    // --- Создаём узлы (через NodeFactory внутри Facade) ---
    auto hostA  = sim.addHost("Host-A", "AA:BB:CC:DD:EE:01", "192.168.1.2");
    auto hostB  = sim.addHost("Host-B", "AA:BB:CC:DD:EE:02", "192.168.2.2");
    auto router = sim.addRouter("Router-1", "AA:BB:CC:DD:EE:FF", "192.168.1.1");

    std::cout << "\n";

    // --- Создаём среды передачи ---
    // LAN1: сегмент между Host-A и Router (10 Мбит/с, задержка 1 мс)
    auto lan1 = sim.addMedium("Medium-LAN1", 10e6, 0.001);

    // LAN2: сегмент между Router и Host-B (10 Мбит/с, задержка 2 мс)
    auto lan2 = sim.addMedium("Medium-LAN2", 10e6, 0.002);

    std::cout << "\n";

    // --- Подключаем узлы к средам ---
    sim.connectNodeToMedium(hostA, lan1);          // Host-A ↔ LAN1

    // Router подключён к обоим сегментам через разные интерфейсы
    sim.connectRouterInterface(router, 0, lan1);   // Router интерфейс-0 ↔ LAN1
    sim.connectRouterInterface(router, 1, lan2);   // Router интерфейс-1 ↔ LAN2

    sim.connectNodeToMedium(hostB, lan2);          // Host-B ↔ LAN2

    std::cout << "\n";

    // =========================================================================
    // 3. Настройка таблицы маршрутизации
    // =========================================================================
    // Маршрут в сеть 192.168.1.0/24 → интерфейс 0
    router->addRoute({"192.168.1.0", "255.255.255.0", "192.168.1.2", 0});
    // Маршрут в сеть 192.168.2.0/24 → интерфейс 1
    router->addRoute({"192.168.2.0", "255.255.255.0", "192.168.2.2", 1});

    std::cout << "[Маршрутизация] Router-1 таблица маршрутов:\n";
    std::cout << "  192.168.1.0/24 → интерфейс 0 (LAN1)\n";
    std::cout << "  192.168.2.0/24 → интерфейс 1 (LAN2)\n\n";

    // =========================================================================
    // 4. Назначение стратегий генерации трафика (Strategy pattern)
    // =========================================================================

    // Host-A: ConstantBitRate — пакет каждые 0.5 секунды (2 пакета/с)
    auto cbrStrategy = std::make_shared<ConstantBitRateStrategy>(
        0.5,   // интервал 500 мс
        128    // размер пакета 128 байт
    );
    hostA->setTrafficStrategy(cbrStrategy);
    // Host-A отправляет Host-B
    hostA->setDestination("192.168.2.2", "AA:BB:CC:DD:EE:FF"); // dst MAC = роутер

    // Host-B: Bursty — 3 пакета с паузой 50 мс, затем пауза 2 секунды
    auto burstyStrategy = std::make_shared<BurstyStrategy>(
        3,     // 3 пакета в пачке
        0.05,  // 50 мс между пакетами в пачке
        2.0,   // 2 секунды паузы между пачками
        64     // размер пакета 64 байта
    );
    hostB->setTrafficStrategy(burstyStrategy);
    // Host-B отправляет Host-A
    hostB->setDestination("192.168.1.2", "AA:BB:CC:DD:EE:FF"); // dst MAC = роутер

    std::cout << "[Трафик] Host-A: CBR (каждые 0.5с, 128 байт)\n";
    std::cout << "[Трафик] Host-B: Bursty (3 пакета по 50мс, пауза 2с, 64 байт)\n\n";

    // =========================================================================
    // 5. Запуск симуляции на 10 секунд
    // =========================================================================
    sim.startSimulation(10.0);

    // =========================================================================
    // 6. Вывод итоговой статистики
    // =========================================================================
    sim.printStats();

    std::cout << "\nПрограмма завершена.\n";
    return 0;
}
