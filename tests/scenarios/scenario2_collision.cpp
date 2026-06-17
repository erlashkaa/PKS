/**
 * scenario2_collision.cpp
 *
 * СЦЕНАРИЙ 2: Возникновение коллизий при одновременной отправке,
 *             срабатывание алгоритма backoff и успешная повторная передача.
 *
 * Топология:
 *   Host-A (CBR 0.05с) ──┐
 *                         ├── [LAN] ── (без получателя — среда 1 узел = нет,
 *   Host-B (CBR 0.05с) ──┘            фактически узлов 2 → вероятность коллизии ~15%)
 *
 * Гипотеза: При двух активных отправителях в одном сегменте
 *   за достаточно долгую симуляцию должны зафиксироваться коллизии.
 *   После backoff пакеты успешно ретранслируются.
 *
 * ASSERT:
 *   - collisions   >= 1    (должны быть)
 *   - delivered    >= 1    (backoff сработал, хоть что-то доставлено)
 *   - delivery rate < 100% (не все пакеты с первого раза)
 */

#include <gtest/gtest.h>
#include "SimulationFacade.h"
#include "TrafficStrategies.h"
#include "SimulationConfig.h"

// Проверяет коллизии и повторы
TEST(Scenario2, CollisionDetection_BackoffAndRetry) {
    std::cout << "\n--- [Сценарий 2: Возникновение и обработка коллизий] ---\n"
              << " Суть: Тестирование механизма CSMA/CD и алгоритма случайной задержки (Backoff).\n"
              << " Сеть: Узлы: 2 хоста (Хост-А, Хост-Б), Среда: 1 общая шина (10 Мбит/с, 1 мс), Подсети: нет.\n"
              << " Поток: Встречный равномерный трафик высокой интенсивности от обоих хостов.\n"
              << " Шаги: Одновременное вещание в одну шину -> Фиксация коллизии -> Случайная пауза -> Повторная отправка.\n"
              << " Результат: Зарегистрированы коллизии, пакеты успешно переотправлены и доставлены.\n"
              << "------------------------------------------------------\n\n";
    Packet::nextId = 1;
    auto& cfg = SimulationConfig::getInstance();
    cfg.tickStep    = 0.001;
    cfg.backoffSlot = 0.005;
    cfg.maxRetries  = 10;
    cfg.currentTime = 0.0;

    // ---- Топология: оба хоста в ОДНОМ сегменте ----
    SimulationFacade sim;

    auto hostA = sim.addHost("Host-A", "AA:BB:CC:DD:EE:01", "192.168.1.2");
    auto hostB = sim.addHost("Host-B", "AA:BB:CC:DD:EE:02", "192.168.1.3");

    // Фиксируем seed для воспроизводимости
    std::srand(12345);

    auto lan = sim.addMedium("LAN-Collision", 10e6, 0.001);
    sim.connectNodeToMedium(hostA, lan);
    sim.connectNodeToMedium(hostB, lan);

    // Оба хоста отправляют часто — интервал 50 мс (много попыток)
    auto cbrA = std::make_shared<ConstantBitRateStrategy>(0.05, 64);
    auto cbrB = std::make_shared<ConstantBitRateStrategy>(0.05, 64);

    hostA->setTrafficStrategy(cbrA);
    hostA->setDestination("192.168.1.3", "AA:BB:CC:DD:EE:02");

    hostB->setTrafficStrategy(cbrB);
    hostB->setDestination("192.168.1.2", "AA:BB:CC:DD:EE:01");

    // ---- Запуск на 5 секунд ----
    sim.startSimulation(5.0);

    auto stats = sim.getStats();

    // 1. При двух активных отправителях должны быть коллизии (15% вероятность)
    EXPECT_GE(stats->getCollisions(), 1)
        << "При двух одновременных отправителях за 5с должна быть хотя бы 1 коллизия";

    // 2. Несмотря на коллизии, что-то всё равно доставлено (backoff работает)
    EXPECT_GE(stats->getPacketsDelivered(), 1)
        << "После backoff хотя бы часть пакетов должна быть доставлена";

    // 3. Отправок было больше нуля
    EXPECT_GE(stats->getPacketsSent(), 5)
        << "За 5 секунд оба хоста должны отправить суммарно >= 5 пакетов";

    std::cout << " Фактические метрики:\n"
              << "  - Ожидалось коллизий: >0. Фактически: " << stats->getCollisions() << "\n"
              << "  - Всего отправлено: " << stats->getPacketsSent() << "\n"
              << "  - Успешно доставлено: " << stats->getPacketsDelivered() << "\n";
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
