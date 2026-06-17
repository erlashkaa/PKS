#include <gtest/gtest.h>
#include "SimulationFacade.h"
#include "TrafficStrategies.h"
#include "SimulationConfig.h"

// Проверяет высокую нагрузку и коллизии
TEST(Scenario4, HeavyLoad_CollisionsAndDrops) {
    std::cout << "\n--- [Сценарий 4: Стресс-тест сети при экстремальной нагрузке] ---\n"
              << " Суть: Проверка поведения среды при лавинообразных коллизиях и жестком лимите попыток.\n"
              << " Сеть: Узлы: 3 хоста (Х1, Х2, Х3), Среда: 1 общая шина (10 Мбит/с, 1 мс), Подсети: нет.\n"
              << " Поток: Взрывной трафик (Bursty) — пачки по 10 пакетов одновременно от всех узлов.\n"
              << " Шаги: Лимит попыток = 1 -> Одновременный запуск -> Исчерпание попыток отправки -> Отбрасывание пакетов.\n"
              << " Результат: Сеть стабильна, коллизии зафиксированы, пакеты безопасно отброшены после лимита.\n"
              << "--------------------------------------------------------------\n\n";
    Packet::nextId = 1;
    auto& cfg = SimulationConfig::getInstance();
    cfg.tickStep = 0.001;
    cfg.backoffSlot = 0.001; // Очень короткий backoff
    cfg.maxRetries = 1;      // Быстрый дроп после 1 коллизии
    cfg.currentTime = 0.0;
    std::srand(12345);

    SimulationFacade sim;
    auto lan = sim.addMedium("LAN", 10e6, 0.001);

    auto h1 = sim.addHost("H1", "MAC1", "10.0.0.1");
    auto h2 = sim.addHost("H2", "MAC2", "10.0.0.2");
    auto h3 = sim.addHost("H3", "MAC3", "10.0.0.3");

    sim.connectNodeToMedium(h1, lan);
    sim.connectNodeToMedium(h2, lan);
    sim.connectNodeToMedium(h3, lan);

    h1->setDestination("10.0.0.2", "MAC2");
    h2->setDestination("10.0.0.1", "MAC1");
    h3->setDestination("10.0.0.1", "MAC1");

    h1->setTrafficStrategy(std::make_shared<BurstyStrategy>(10, 0.0, 1.0, 1000));
    h2->setTrafficStrategy(std::make_shared<BurstyStrategy>(10, 0.0, 1.0, 1000));
    h3->setTrafficStrategy(std::make_shared<BurstyStrategy>(10, 0.0, 1.0, 1000));

    sim.startSimulation(3.0);
    auto stats = sim.getStats();

    // Мы ожидаем много коллизий
    EXPECT_GT(stats->getCollisions(), 0) << "Должны быть коллизии при высокой нагрузке";

    int totalSent = h1->getSentCount() + h2->getSentCount() + h3->getSentCount();
    int totalDropped = h1->getDroppedCount() + h2->getDroppedCount() + h3->getDroppedCount();

    // Мы ожидаем, что при таком сильном залпе и лимите 1 попытки часть пакетов обязательно будет сброшена!
    EXPECT_GT(totalDropped, 0) << "При лимите maxRetries=1 часть пакетов должна быть отброшена!";

    std::cout << " Фактические метрики:\n"
              << "  - Ожидалось коллизий: >0. Фактически: " << stats->getCollisions() << "\n"
              << "  - Всего сгенерировано пакетов: " << (totalSent + totalDropped) << "\n"
              << "  - Успешно отправлено и доставлено: " << totalSent << "\n"
              << "  - Отброшено по лимиту попыток (Drop): " << totalDropped << "\n";
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
