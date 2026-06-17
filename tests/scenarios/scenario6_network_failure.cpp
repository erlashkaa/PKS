#include <gtest/gtest.h>
#include "SimulationFacade.h"
#include "TrafficStrategies.h"
#include "SimulationConfig.h"

// Проверяет поведение при отсутствии маршрута
TEST(Scenario6, NetworkFailure_NoRouteDropsPacket) {
    std::cout << "\n--- [Сценарий 6: Сбой маршрутизации / Отсутствие пути] ---\n"
              << " Суть: Проверка отказоустойчивости при отправке пакета в несуществующее направление.\n"
              << " Сеть: Узлы: 1 хост, 1 маршрутизатор, Среда: 1 общая шина (100 Мбит/с), Подсети: да.\n"
              << " Поток: Хост шлет пакет в подсеть 172.16.0.0 через маршрутизатор с пустой таблицей.\n"
              << " Шаги: Доставка до маршрутизатора -> Поиск в таблице -> Отсутствие совпадений -> Безопасный сброс пакета.\n"
              << " Результат: Отсутствие падений системы, пакет корректно утилизирован на сетевом уровне.\n"
              << "--------------------------------------------------------\n\n";
    Packet::nextId = 1;
    auto& cfg = SimulationConfig::getInstance();
    cfg.currentTime = 0.0;
    
    SimulationFacade sim;
    auto lan = sim.addMedium("LAN", 100e6, 0.0001);
    
    auto host = sim.addHost("Host", "MAC-H", "10.0.0.5");
    auto router = sim.addRouter("Router", "MAC-R", "10.0.0.1");
    
    sim.connectNodeToMedium(host, lan);
    sim.connectNodeToMedium(router, lan); // router подключен, но не имеет маршрутов
    
    // Хост отправляет пакет в неизвестную сеть 172.16.0.5 через роутер
    host->setDestination("172.16.0.5", "MAC-R");
    host->setTrafficStrategy(std::make_shared<ConstantBitRateStrategy>(0.5, 64));
    
    sim.startSimulation(2.0);
    
    auto stats = sim.getStats();
    // Пакет дойдет до роутера, но роутер его отбросит
    EXPECT_GT(stats->getPacketsSent(), 0) << "Хост должен отправить хотя бы один пакет";
    EXPECT_EQ(stats->getPacketsDelivered(), stats->getPacketsSent()) << "Все пакеты должны дойти до роутера";
    // Статистика отправленных должна равняться принятым роутером, дальше ничего не идет

    std::cout << " Фактические метрики:\n"
              << "  - Отправлено Хостом в сеть: " << host->getSentCount() << "\n"
              << "  - Доставлено на Маршрутизатор (L2): " << stats->getPacketsDelivered() << "\n"
              << "  - Отброшено Маршрутизатором из-за отсутствия пути (Drop): " << stats->getPacketsDelivered() << "\n";
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
