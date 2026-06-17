/**
 * scenario3_routing.cpp
 *
 * СЦЕНАРИЙ 3: Маршрутизация пакета из одной подсети в другую
 *             через маршрутизатор на основе статической таблицы маршрутизации.
 *
 * Топология:
 *   Host-A (192.168.1.2) ── [LAN1] ── Router-1 ── [LAN2] ── Host-B (192.168.2.2)
 *
 * Гипотеза: Пакет из 192.168.1.0/24 → 192.168.2.0/24 должен пройти
 *   через роутер и быть доставлен Host-B.
 *
 * ASSERT:
 *   - Host-B.receivedCount >= 1          (пакеты реально дошли)
 *   - stats(LAN2).delivered >= 1         (роутер переслал в LAN2)
 *   - Host-A.sentCount >= 1              (Host-A отправлял)
 */

#include <gtest/gtest.h>
#include "SimulationFacade.h"
#include "TrafficStrategies.h"
#include "SimulationConfig.h"
#include "StatisticsCollector.h"
// Проверяет маршрутизацию между подсетями
TEST(Scenario3, StaticRouting_PacketCrossesSubnets) {
    std::cout << "\n--- [Сценарий 3: Маршрутизация между подсетями] ---\n"
              << " Суть: Проверка прохождения пакетов через сетевой уровень маршрутизатора.\n"
              << " Сеть: Узлы: Хост-А (сеть 1), Хост-Б (сеть 2) и Маршрутизатор, Среда: 2 изолированные шины (10 Мбит/с), Подсети: да.\n"
              << " Поток: Хост-А отправляет пакеты Хосту-Б во вторую подсеть через шлюз по умолчанию.\n"
              << " Шаги: Анализ IP назначения -> Определение маршрута -> Пересылка из первой шины во вторую.\n"
              << " Результат: Пакеты успешно преодолевают границы подсетей и доходят до адресата.\n"
              << "----------------------------------------------\n\n";
    Packet::nextId = 1;
    auto& cfg = SimulationConfig::getInstance();
    cfg.tickStep    = 0.001;
    cfg.backoffSlot = 0.005;
    cfg.maxRetries  = 10;
    cfg.currentTime = 0.0;
    std::srand(99999);   // seed для стабильного поведения коллизий

    // ---- Топология: классическая маршрутизируемая сеть ----
    SimulationFacade sim;

    auto hostA  = sim.addHost("Host-A",   "AA:BB:CC:DD:EE:01", "192.168.1.2");
    auto hostB  = sim.addHost("Host-B",   "AA:BB:CC:DD:EE:02", "192.168.2.2");
    auto router = sim.addRouter("Router", "AA:BB:CC:DD:EE:FF", "192.168.1.1");

    // Быстрые среды, минимальная задержка (для теста)
    auto lan1 = sim.addMedium("LAN1", 100e6, 0.0001);
    auto lan2 = sim.addMedium("LAN2", 100e6, 0.0001);

    // Отдельные коллекторы для LAN1 и LAN2
    auto statsLan1 = std::make_shared<StatisticsCollector>();
    lan1->subscribe(statsLan1);
    auto statsLan2 = std::make_shared<StatisticsCollector>();
    lan2->subscribe(statsLan2);

    sim.connectNodeToMedium(hostA,    lan1);
    sim.connectRouterInterface(router, 0, lan1);
    sim.connectRouterInterface(router, 1, lan2);
    sim.connectNodeToMedium(hostB,    lan2);

    // ---- Таблица маршрутизации ----
    router->addRoute({"192.168.1.0", "255.255.255.0", "192.168.1.2", 0});
    router->addRoute({"192.168.2.0", "255.255.255.0", "192.168.2.2", 1});

    // ---- CBR: Host-A отправляет Host-B через роутер ----
    auto cbr = std::make_shared<ConstantBitRateStrategy>(0.5, 64);
    hostA->setTrafficStrategy(cbr);
    // dst MAC = роутер (следующий hop на L2)
    hostA->setDestination("192.168.2.2", "AA:BB:CC:DD:EE:FF");

    // ---- Симуляция 5 секунд ----
    sim.startSimulation(5.0);

    // ---- ASSERTIONS ----

    // 1. Host-A отправил пакеты
    EXPECT_GE(hostA->getSentCount(), 1)
        << "Host-A должен отправить хотя бы 1 пакет";

    // 2. Роутер переслал хотя бы один пакет в LAN2
    EXPECT_GE(statsLan2->getPacketsSent(), 1)
        << "Router должен переслать пакет в LAN2 (192.168.2.0/24)";

    // 3. Host-B (в LAN2) получил пакеты
    EXPECT_GE(hostB->getReceivedCount(), 1)
        << "Host-B (192.168.2.2) должен получить пакеты от роутера";

    // 4. В LAN2 всё доставлено (нет коллизий, один получатель)
    EXPECT_EQ(statsLan2->getPacketsSent(), statsLan2->getPacketsDelivered())
        << "В LAN2 коллизий нет, доставка должна быть 100%";

    // 5. Число пакетов, полученных Host-B, соответствует пересланным в LAN2
    EXPECT_EQ(statsLan2->getPacketsDelivered(), hostB->getReceivedCount())
        << "Все пакеты, отправленные в LAN2, должны быть получены Host-B";

    std::cout << " Фактические метрики:\n"
              << "  - Отправлено Хостом-А в LAN1: " << hostA->getSentCount() << "\n"
              << "  - Доставлено на Роутер из LAN1: " << statsLan1->getPacketsDelivered() << "\n"
              << "  - Переслано Роутером в LAN2: " << statsLan2->getPacketsSent() << "\n"
              << "  - Получено Хостом-Б из LAN2: " << hostB->getReceivedCount() << "\n";
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    auto& listeners = ::testing::UnitTest::GetInstance()->listeners();
    delete listeners.Release(listeners.default_result_printer());
    int result = RUN_ALL_TESTS();
    if (result == 0) {
        std::cout << "[УСПЕШНО] Сценарий выполнен без ошибок!\n\n";
    } else {
        std::cout << "[ОШИБКА] В сценарии обнаружены сбои!\n\n";
    }
    return result;
}
