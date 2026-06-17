#include <gtest/gtest.h>
#include "SimulationFacade.h"
#include "TrafficStrategies.h"
#include "PoissonStrategy.h"
#include "SimulationConfig.h"

// Проверяет смешанный трафик
TEST(Scenario5, MixedTraffic_PoissonAndCBR) {
    std::cout << "\n--- [Сценарий 5: Смешанный тип сетевого трафика] ---\n"
              << " Суть: Анализ стабильности симулятора при совмещении регулярного и случайного трафика.\n"
              << " Сеть: Узлы: 2 отправителя, 1 получатель, Среда: 1 общая шина (100 Мбит/с, 0.1 мс), Подсети: нет.\n"
              << " Поток: Равномерный (CBR) + Случайный Пуассоновский трафик на один узел-приемник.\n"
              << " Шаги: Генерация Пуассоновских интервалов -> Слияние потоков в канале -> Успешный прием получателем.\n"
              << " Результат: Все типы трафика успешно обработаны без сбоев в планировщике событий.\n"
              << "------------------------------------------------\n\n";
    Packet::nextId = 1;
    auto& cfg = SimulationConfig::getInstance();
    cfg.tickStep = 0.001;
    cfg.backoffSlot = 0.005;
    cfg.maxRetries = 10;
    cfg.currentTime = 0.0;
    std::srand(42);

    SimulationFacade sim;
    auto lan = sim.addMedium("LAN", 100e6, 0.0001);

    auto cbrHost = sim.addHost("CBR-Host", "MAC-CBR", "192.168.1.10");
    auto poissonHost = sim.addHost("Poisson-Host", "MAC-POI", "192.168.1.20");
    auto receiver = sim.addHost("Receiver", "MAC-RCV", "192.168.1.30");

    sim.connectNodeToMedium(cbrHost, lan);
    sim.connectNodeToMedium(poissonHost, lan);
    sim.connectNodeToMedium(receiver, lan);

    cbrHost->setDestination("192.168.1.30", "MAC-RCV");
    poissonHost->setDestination("192.168.1.30", "MAC-RCV");

    cbrHost->setTrafficStrategy(std::make_shared<ConstantBitRateStrategy>(0.1, 512));
    poissonHost->setTrafficStrategy(std::make_shared<PoissonStrategy>(10.0, 512, 42));

    sim.startSimulation(2.0);

    EXPECT_GT(receiver->getReceivedCount(), 0) << "Receiver должен получить пакеты";

    auto stats = sim.getStats();
    std::cout << " Фактические метрики:\n"
              << "  - Отправлено (постоянный поток CBR): " << cbrHost->getSentCount() << "\n"
              << "  - Отправлено (случайный поток Poisson): " << poissonHost->getSentCount() << "\n"
              << "  - Всего успешно получено Приемником: " << receiver->getReceivedCount() << "\n"
              << "  - Зарегистрировано коллизий в канале: " << stats->getCollisions() << "\n";
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
