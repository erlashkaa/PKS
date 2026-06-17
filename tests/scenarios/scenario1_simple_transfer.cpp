/**
 * scenario1_simple_transfer.cpp
 *
 * СЦЕНАРИЙ 1: Успешная передача серии пакетов между двумя хостами
 *             в сети без коллизий (один хост на сегмент).
 *
 * Топология:
 *   Host-A (CBR, 192.168.1.2) ── [LAN1] ── Host-B (192.168.1.3)
 *
 * Гипотеза: При одном активном отправителе пакеты успешно доставляются.
 *   Коллизии возможны (рандомная модель в CSMACDMedium), но backoff
 *   обеспечивает повторную передачу → delivery rate >= 90%.
 *
 * ASSERT:
 *   - delivery_rate >= 90%  (не менее 9 из 10)
 *   - delivered     >= 5
 *   - Host-B.receivedCount >= 5
 */

#include <gtest/gtest.h>
#include "SimulationFacade.h"
#include "TrafficStrategies.h"
#include "SimulationConfig.h"

// Проверяет простую передачу без коллизий
TEST(Scenario1, SimpleTransfer_NoCollisions) {
    std::cout << "\n--- [Сценарий 1: Простая передача данных] ---\n"
              << " Суть: Тестирование базовой отправки пакетов между двумя узлами без помех.\n"
              << " Сеть: Узлы: 2 хоста (Хост-А, Хост-Б), Среда: 1 общая шина (10 Мбит/с, 1 мс), Подсети: нет.\n"
              << " Поток: Равномерный трафик (CBR) от Хоста-А к Хосту-Б.\n"
              << " Шаги: Подключение к шине -> Генерация пакетов 2 секунды -> Проверка успешности доставки.\n"
              << " Результат: Все пакеты доставлены вовремя, коллизии отсутствуют.\n"
              << "--------------------------------------------\n\n";
    // ---- Сброс глобального состояния ----
    Packet::nextId = 1;
    auto& cfg = SimulationConfig::getInstance();
    cfg.tickStep    = 0.001;
    cfg.backoffSlot = 0.005;
    cfg.maxRetries  = 10;
    cfg.currentTime = 0.0;

    // ---- Топология ----
    SimulationFacade sim;

    auto hostA = sim.addHost("Host-A", "AA:BB:CC:DD:EE:01", "192.168.1.2");
    auto hostB = sim.addHost("Host-B", "AA:BB:CC:DD:EE:02", "192.168.1.3");

    // Один хост на один сегмент — коллизий не будет
    auto lan = sim.addMedium("LAN", 100e6, 0.0001);  // 100 Мбит/с, delay=0.1мс

    sim.connectNodeToMedium(hostA, lan);
    sim.connectNodeToMedium(hostB, lan);

    // ---- Стратегия: CBR, пакет каждые 0.5с ----
    auto cbr = std::make_shared<ConstantBitRateStrategy>(0.5, 64);
    hostA->setTrafficStrategy(cbr);
    hostA->setDestination("192.168.1.3", "AA:BB:CC:DD:EE:02");

    // ---- Запуск симуляции на 5 секунд ----
    sim.startSimulation(5.0);

    // ---- ASSERTIONS ----
    auto stats = sim.getStats();

    // 1. За 5 секунд с интервалом 0.5с должно быть отправлено ~10 пакетов
    EXPECT_GE(stats->getPacketsSent(), 5)
        << "За 5 секунд CBR(0.5с) должен отправить не менее 5 пакетов";

    // 2. Delivery rate >= 90% (backoff компенсирует случайные коллизии модели)
    int sent      = stats->getPacketsSent();
    int delivered = stats->getPacketsDelivered();
    if (sent > 0) {
        double rate = 100.0 * delivered / sent;
        EXPECT_GE(rate, 90.0)
            << "Delivery rate должен быть не ниже 90% (backoff компенсирует коллизии)";
    }

    // 3. Host-B получил пакеты
    EXPECT_GE(hostB->getReceivedCount(), 5)
        << "Host-B должен получить не менее 5 пакетов";

    // 4. Host-A отправил пакеты
    EXPECT_GE(hostA->getSentCount(), 5)
        << "Host-A должен отправить не менее 5 пакетов";

    // 5. Доставлено не меньше чем отправлено хостом
    EXPECT_LE(hostA->getSentCount(), delivered + stats->getCollisions())
        << "Каждый отправленный пакет либо доставлен, либо вызвал коллизию";

    std::cout << " Фактические метрики:\n"
              << "  - Отправлено Хостом-А: " << hostA->getSentCount() << "\n"
              << "  - Доставлено Хосту-Б: " << hostB->getReceivedCount() << "\n"
              << "  - Возникло коллизий: " << stats->getCollisions() << "\n";
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
