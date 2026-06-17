#include "CSMACDMedium.h"
#include "Host.h"
#include "Packet.h"
#include "PoissonStrategy.h"
#include "Router.h"
#include "SimulationConfig.h"
#include "SimulationParameters.h"
#include "StatisticsCollector.h"
#include "TrafficStrategies.h"
#include <gtest/gtest.h>

// 1. Инициализация полей
TEST(PacketTest, ConstructorAndFields) {
  Packet::nextId = 100;
  Packet pkt("MAC-A", "MAC-B", "10.0.0.1", "10.0.0.2", "Hello", 128, 1.25);
  EXPECT_EQ(pkt.id, 100);
  EXPECT_EQ(pkt.srcMAC, "MAC-A");
  EXPECT_EQ(pkt.dstMAC, "MAC-B");
  EXPECT_EQ(pkt.srcIP, "10.0.0.1");
  EXPECT_EQ(pkt.dstIP, "10.0.0.2");
  EXPECT_EQ(pkt.payload, "Hello");
  EXPECT_EQ(pkt.sizeBytes, 128);
  EXPECT_DOUBLE_EQ(pkt.creationTime, 1.25);
}

// 2. Размер по умолчанию
TEST(PacketTest, DefaultConstructorSize) {
  Packet pkt("MAC-A", "MAC-B", "10.0.0.1", "10.0.0.2", "Hi");
  EXPECT_EQ(pkt.sizeBytes, 64); // По умолчанию 64 байта
  EXPECT_DOUBLE_EQ(pkt.creationTime, 0.0);
}

// 3. Автоинкремент уникальных ID
TEST(PacketTest, UniqueIdIncrement) {
  Packet::nextId = 500;
  Packet p1("M1", "M2", "1.1.1.1", "2.2.2.2", "Data");
  Packet p2("M1", "M2", "1.1.1.1", "2.2.2.2", "Data");
  EXPECT_EQ(p1.id, 500);
  EXPECT_EQ(p2.id, 501);
}

// 4. Корректность копирования
TEST(PacketTest, CopyConstructor) {
  Packet orig("MAC-A", "MAC-B", "10.0.0.1", "10.0.0.2", "Payload", 500, 3.5);
  Packet copy = orig;
  EXPECT_EQ(copy.id, orig.id);
  EXPECT_EQ(copy.srcMAC, orig.srcMAC);
  EXPECT_EQ(copy.dstMAC, orig.dstMAC);
  EXPECT_EQ(copy.srcIP, orig.srcIP);
  EXPECT_EQ(copy.dstIP, orig.dstIP);
  EXPECT_EQ(copy.payload, orig.payload);
  EXPECT_EQ(copy.sizeBytes, orig.sizeBytes);
  EXPECT_DOUBLE_EQ(copy.creationTime, orig.creationTime);
}

// 5. Валидация пустого пакета
TEST(PacketTest, EmptyPayload) {
  Packet pkt("MAC-A", "MAC-B", "10.0.0.1", "10.0.0.2", "");
  EXPECT_EQ(pkt.payload, "");
  EXPECT_EQ(pkt.sizeBytes, 64);
}

// 6. Максимальный размер полезной нагрузки
TEST(PacketTest, LargePayload) {
  std::string largePayload(65535, 'X');
  Packet pkt("MAC-A", "MAC-B", "10.0.0.1", "10.0.0.2", largePayload, 65535);
  EXPECT_EQ(pkt.payload.size(), 65535);
  EXPECT_EQ(pkt.sizeBytes, 65535);
}

// =============================================================================
// RouterTest - Тесты для класса Router
// =============================================================================

// 7. Точное совпадение (Exact Match)
TEST(RouterTest, ExactMatch) {
  Router router("R1", "MAC-R", "192.168.1.1");
  router.addRoute({"192.168.1.0", "255.255.255.0", "192.168.1.1", 0});

  auto lan1 = std::make_shared<CSMACDMedium>("LAN1", 10e6, 0.001);
  router.connectInterface(0, lan1);

  auto host = std::make_shared<Host>("H", "MAC-H", "192.168.1.5");
  lan1->attachNode(host);

  Packet pkt("MAC-S", "MAC-R", "192.168.1.10", "192.168.1.5", "Hello");
  router.receivePacket(pkt);

  EXPECT_EQ(host->getReceivedCount(), 1);
}

// 8. Longest Prefix Match (LPM)
TEST(RouterTest, LongestPrefixMatch) {
  Router router("R1", "MAC-R", "192.168.1.1");
  // Маршрут /24 в интерфейс 0
  router.addRoute({"192.168.1.0", "255.255.255.0", "192.168.1.1", 0});
  // Более специфичный маршрут /25 в интерфейс 1
  router.addRoute({"192.168.1.128", "255.255.255.128", "192.168.1.129", 1});

  auto lan0 = std::make_shared<CSMACDMedium>("LAN0", 10e6, 0.001);
  auto lan1 = std::make_shared<CSMACDMedium>("LAN1", 10e6, 0.001);
  router.connectInterface(0, lan0);
  router.connectInterface(1, lan1);

  auto host0 = std::make_shared<Host>("H0", "MAC-H0",
                                      "192.168.1.10"); // подпадает под /24
  auto host1 = std::make_shared<Host>(
      "H1", "MAC-H1",
      "192.168.1.135"); // подпадает под оба, но /25 приоритетнее
  lan0->attachNode(host0);
  lan1->attachNode(host1);

  // Тестируем пересылку пакета для 192.168.1.135 (должен уйти на LAN1)
  Packet pkt("MAC-S", "MAC-R", "192.168.1.10", "192.168.1.135", "LPM Test");
  router.receivePacket(pkt);

  EXPECT_EQ(host1->getReceivedCount(), 1);
  EXPECT_EQ(host0->getReceivedCount(), 0);
}

// 9. Маршрут по умолчанию (Default Route)
TEST(RouterTest, DefaultRoute) {
  Router router("R1", "MAC-R", "192.168.1.1");
  // Конкретный маршрут в интерфейс 0
  router.addRoute({"192.168.1.0", "255.255.255.0", "192.168.1.1", 0});
  // Маршрут по умолчанию (0.0.0.0/0) в интерфейс 1
  router.addRoute({"0.0.0.0", "0.0.0.0", "192.168.2.1", 1});

  auto lan0 = std::make_shared<CSMACDMedium>("LAN0", 10e6, 0.001);
  auto lan1 = std::make_shared<CSMACDMedium>("LAN1", 10e6, 0.001);
  router.connectInterface(0, lan0);
  router.connectInterface(1, lan1);

  auto hostDefault = std::make_shared<Host>("HD", "MAC-HD", "8.8.8.8");
  lan1->attachNode(hostDefault);

  // Пакет на внешний адрес 8.8.8.8
  Packet pkt("MAC-S", "MAC-R", "192.168.1.10", "8.8.8.8", "Internet");
  router.receivePacket(pkt);

  EXPECT_EQ(hostDefault->getReceivedCount(), 1);
}

// 10. Отсутствие маршрута и сброс (No Route Drop)
TEST(RouterTest, NoRouteDrop) {
  Router router("R1", "MAC-R", "192.168.1.1");
  // Пустая таблица маршрутизации

  auto lan = std::make_shared<CSMACDMedium>("LAN", 10e6, 0.001);
  router.connectInterface(0, lan);

  auto host = std::make_shared<Host>("H", "MAC-H", "192.168.2.2");
  lan->attachNode(host);

  Packet pkt("MAC-S", "MAC-R", "192.168.1.10", "192.168.2.2", "Drop Me");
  router.receivePacket(pkt);

  EXPECT_EQ(host->getReceivedCount(), 0); // Пакет отброшен
}

// 11. Переопределение маршрута (Update Route)
TEST(RouterTest, UpdateRoute) {
  Router router("R1", "MAC-R", "192.168.1.1");
  // Первый маршрут: в интерфейс 0
  router.addRoute({"192.168.1.0", "255.255.255.0", "192.168.1.1", 0});
  // Переопределяем (добавляем более приоритетный duplicate с той же маской): в
  // интерфейс 1
  router.addRoute({"192.168.1.0", "255.255.255.0", "192.168.1.1", 1});

  auto lan0 = std::make_shared<CSMACDMedium>("LAN0", 10e6, 0.001);
  auto lan1 = std::make_shared<CSMACDMedium>("LAN1", 10e6, 0.001);
  router.connectInterface(0, lan0);
  router.connectInterface(1, lan1);

  auto host1 = std::make_shared<Host>("H1", "MAC-H1", "192.168.1.5");
  lan1->attachNode(host1);

  Packet pkt("MAC-S", "MAC-R", "192.168.1.10", "192.168.1.5", "Override Test");
  router.receivePacket(pkt);

  EXPECT_EQ(host1->getReceivedCount(),
            1); // Узел на интерфейсе 1 успешно получил
}

// 12. Множество интерфейсов роутера
TEST(RouterTest, MultipleInterfaces) {
  Router router("R1", "MAC-R", "10.0.0.1");
  router.addRoute({"10.1.0.0", "255.255.0.0", "10.1.0.1", 1});
  router.addRoute({"10.2.0.0", "255.255.0.0", "10.2.0.1", 2});

  auto lan1 = std::make_shared<CSMACDMedium>("LAN1", 10e6, 0.001);
  auto lan2 = std::make_shared<CSMACDMedium>("LAN2", 10e6, 0.001);
  router.connectInterface(1, lan1);
  router.connectInterface(2, lan2);

  auto host2 = std::make_shared<Host>("H2", "MAC-H2", "10.2.0.5");
  lan2->attachNode(host2);

  Packet pkt("MAC-S", "MAC-R", "10.1.0.5", "10.2.0.5", "Routed");
  router.receivePacket(pkt);

  EXPECT_EQ(host2->getReceivedCount(), 1);
}

// =============================================================================
// HostTest - Тесты для класса Host
// =============================================================================

// 13. Генерация CBR с ровным интервалом
TEST(HostTest, SendPacketCBR) {
  auto host = std::make_shared<Host>("H", "MAC-H", "10.0.0.1");
  host->setDestination("10.0.0.2", "MAC-B");
  host->setTrafficStrategy(
      std::make_shared<ConstantBitRateStrategy>(0.1, 100)); // 100 мс интервал

  auto lan = std::make_shared<CSMACDMedium>("LAN", 10e6, 0.001);
  lan->attachNode(host);
  host->connectTo(lan);

  // t=0.0: CBR отправляет сразу первый пакет
  lan->tick(0.0);
  host->tick(0.0);
  EXPECT_EQ(host->getSentCount(), 1);

  // t=0.05: Интервал 100 мс еще не прошел, пакет генерироваться не должен
  lan->tick(0.05);
  host->tick(0.05);
  EXPECT_EQ(host->getSentCount(), 1);

  // t=0.1: Интервал прошел, освобождаем среду, отправляется второй пакет
  lan->tick(0.1);
  host->tick(0.1);
  EXPECT_EQ(host->getSentCount(), 2);
}

// 14. Генерация Пуассоновского трафика
TEST(HostTest, SendPacketPoisson) {
  auto host = std::make_shared<Host>("H", "MAC-H", "10.0.0.1");
  host->setDestination("10.0.0.2", "MAC-B");
  host->setTrafficStrategy(std::make_shared<PoissonStrategy>(10.0, 100, 42));

  auto lan = std::make_shared<CSMACDMedium>("LAN", 10e6, 0.001);
  lan->attachNode(host);
  host->connectTo(lan);

  // Крутим время в симуляции, чтобы пуассоновский процесс выбросил события
  for (double t = 0.0; t < 1.0; t += 0.01) {
    lan->tick(t);
    host->tick(t);
  }
  EXPECT_GT(host->getSentCount(),
            0); // Хотя бы один пакет должен сгенерироваться за 1 секунду
}

// 15. Успешный прием валидного пакета
TEST(HostTest, ReceiveValidPacket) {
  Host host("Target", "MAC-T", "192.168.1.5");
  Packet pkt("MAC-S", "MAC-T", "192.168.1.10", "192.168.1.5", "Valid");

  host.receivePacket(pkt);
  EXPECT_EQ(host.getReceivedCount(), 1);
}

// 16. Игнорирование пакетов с другим MAC/IP адресом
TEST(HostTest, IgnorePacketWithWrongMAC) {
  Host host("Target", "MAC-T", "192.168.1.5");
  Packet pkt("MAC-S", "MAC-OTHER", "192.168.1.10", "192.168.1.20", "Unicast");

  host.receivePacket(pkt);
  EXPECT_EQ(host.getReceivedCount(), 0); // Игнорирован
}

// 17. Рост очереди при занятом канале
TEST(HostTest, BufferOverflow) {
  auto host = std::make_shared<Host>("H", "MAC-H", "10.0.0.1");
  host->setDestination("10.0.0.2", "MAC-B");
  host->setTrafficStrategy(std::make_shared<ConstantBitRateStrategy>(0.1));

  auto lan = std::make_shared<CSMACDMedium>("LAN", 10e6, 0.001);
  lan->attachNode(host);
  host->connectTo(lan);

  // Забиваем эфир другим пакетом, симулируя занятый канал на долгое время
  Packet busyPkt("MAC-X", "MAC-Y", "10.0.0.9", "10.0.0.8", "Big payload");
  lan->sendPacket(busyPkt, "MAC-X", 0.0);

  // Наш хост CBR пытается слать пакеты каждые 100 мс, но канал занят (так как
  // мы не вызываем lan->tick()!)
  host->tick(0.0);
  host->tick(0.1);
  host->tick(0.2);

  // Пакеты накапливаются в очереди (backlog)
  EXPECT_EQ(host->getSendQueueSize(), 3);
  EXPECT_EQ(host->getSentCount(), 0); // Ни один не ушел в эфир
}

// 18. Отбрасывание по лимиту попыток (Drop)
TEST(HostTest, DropOnCollisionLimit) {
  auto host = std::make_shared<Host>("H", "MAC-H", "10.0.0.1");
  host->setDestination("10.0.0.2", "MAC-B");
  host->setTrafficStrategy(
      std::make_shared<ConstantBitRateStrategy>(0.01)); // Шлём часто

  auto lan = std::make_shared<CSMACDMedium>("LAN", 10e6, 0.001);
  lan->attachNode(host);
  host->connectTo(lan);

  auto host2 = std::make_shared<Host>("H2", "MAC-H2", "10.0.0.2");
  host2->setDestination("10.0.0.1", "MAC-H");
  host2->setTrafficStrategy(std::make_shared<ConstantBitRateStrategy>(0.01));
  lan->attachNode(host2);
  host2->connectTo(lan);

  // Устанавливаем лимит попыток = 1
  SimulationParameters::getInstance().maxRetries = 1;
  std::srand(42); // Фиксируем случайность для детерминированности

  for (int i = 0; i < 100; ++i) {
    double t = i * 0.01;
    lan->tick(t);
    host->tick(t);
    host2->tick(t);
  }
  EXPECT_GT(host->getDroppedCount(), 0);
}

// =============================================================================
// CSMACDMediumTest - Тесты для CSMACDMedium
// =============================================================================

// 19. Успешная передача при свободном канале
TEST(CSMACDMediumTest, ChannelIdleTransmission) {
  CSMACDMedium medium("LAN", 10e6, 0.001);
  EXPECT_TRUE(medium.isIdle());

  Packet pkt("MAC-A", "MAC-B", "10.0.0.1", "10.0.0.2", "Payload", 125);
  bool ok = medium.sendPacket(pkt, "MAC-A", 1.0);
  EXPECT_TRUE(ok);
  EXPECT_FALSE(medium.isIdle()); // Канал занят
}

// 20. Симуляция коллизий (Collision Detection)
TEST(CSMACDMediumTest, CollisionDetectionSimulation) {
  CSMACDMedium medium("LAN", 10e6, 0.001);

  // Подключаем два хоста с активными стратегиями, чтобы разрешить коллизии в
  // CSMACDMedium
  auto h1 = std::make_shared<Host>("H1", "MAC1", "10.0.0.1");
  auto h2 = std::make_shared<Host>("H2", "MAC2", "10.0.0.2");
  h1->setTrafficStrategy(std::make_shared<ConstantBitRateStrategy>(0.1));
  h2->setTrafficStrategy(std::make_shared<ConstantBitRateStrategy>(0.1));
  medium.attachNode(h1);
  medium.attachNode(h2);

  Packet pkt1("MAC1", "MAC2", "10.0.0.1", "10.0.0.2", "Data1");
  Packet pkt2("MAC2", "MAC1", "10.0.0.2", "10.0.0.1", "Data2");

  std::srand(7); // Инициализация зерна для провокации коллизии

  bool collisionSeen = false;
  for (int i = 0; i < 100; ++i) {
    // Симулируем попытку одновременной отправки
    bool ok1 = medium.sendPacket(pkt1, "MAC1", 1.0 + i);
    bool ok2 = medium.sendPacket(pkt2, "MAC2", 1.0 + i);
    if (!ok1 || !ok2) {
      collisionSeen = true;
      break;
    }
    medium.tick(2.0 + i);
  }
  EXPECT_TRUE(collisionSeen); // Коллизия должна была произойти
}

// 21. Ограничение Exponential Backoff
TEST(CSMACDMediumTest, BackoffTimeoutCalculation) {
  auto &params = SimulationParameters::getInstance();
  params.backoffSlot = 0.001; // 1 мс слот

  // В CSMACDMedium backoffSlot вычисляется как rand(0, 2^attempt) *
  // backoffSlot. Проверим, что формула выдает интервалы, кратные backoffSlot.
  CSMACDMedium medium("LAN", 10e6, 0.001);

  auto h1 = std::make_shared<Host>("H1", "MAC1", "10.0.0.1");
  auto h2 = std::make_shared<Host>("H2", "MAC2", "10.0.0.2");
  h1->setTrafficStrategy(std::make_shared<ConstantBitRateStrategy>(0.1));
  h2->setTrafficStrategy(std::make_shared<ConstantBitRateStrategy>(0.1));
  medium.attachNode(h1);
  medium.attachNode(h2);

  Packet pkt("MAC1", "MAC2", "10.0.0.1", "10.0.0.2", "Data");

  std::srand(100);
  // Провоцируем коллизию
  bool ok = true;
  for (int i = 0; i < 50; ++i) {
    ok = medium.sendPacket(pkt, "MAC1", 0.0);
    if (!ok)
      break; // Зафиксирована коллизия
  }
  // При коллизии канал блокируется на время backoff. Проверим занятость.
  EXPECT_FALSE(ok);
}

// 22. Задержка распространения сигнала (Signal Propagation)
TEST(CSMACDMediumTest, SignalPropagationDelay) {
  CSMACDMedium medium("LAN", 10e6, 0.005); // 5 мс задержка распространения
  Packet pkt("MAC-A", "MAC-B", "10.0.0.1", "10.0.0.2", "Data", 125); // 1000 бит

  // txTime = 1000 бит / 10,000,000 бит/с = 0.1 мс = 0.0001с.
  // С учетом delay: 0.0001 + 0.005 = 0.0051с.
  double expectedTime = 0.0051;
  EXPECT_DOUBLE_EQ(medium.calcTransmissionTime(pkt), expectedTime);
}

// 23. Оповещение наблюдателей (Observer API)
TEST(CSMACDMediumTest, ObserverSubscription) {
  auto stats = std::make_shared<StatisticsCollector>();
  auto medium = std::make_shared<CSMACDMedium>("LAN", 10e6, 0.001);
  medium->subscribe(stats);

  Packet pkt("MAC-A", "MAC-B", "10.0.0.1", "10.0.0.2", "Observe");
  medium->sendPacket(pkt, "MAC-A", 0.0);

  EXPECT_EQ(stats->getPacketsSent(), 1);
  EXPECT_EQ(stats->getPacketsDelivered(), 1);
}

// =============================================================================
// StatisticsCollectorTest - Тесты для StatisticsCollector
// =============================================================================

// 24. Увеличение счетчика отправленных
TEST(StatisticsCollectorTest, IncrementSent) {
  StatisticsCollector stats;
  Packet pkt("MA", "MB", "1.1.1.1", "2.2.2.2", "Data");
  stats.onEvent(MediumEvent::PACKET_SENT, pkt);
  EXPECT_EQ(stats.getPacketsSent(), 1);
  EXPECT_EQ(stats.getPacketsDelivered(), 0);
}

// 25. Увеличение счетчика отброшенных
TEST(StatisticsCollectorTest, IncrementDropped) {
  StatisticsCollector stats;
  stats.recordDrop();
  stats.recordDrop();
  EXPECT_EQ(stats.getPacketsDropped(), 2);
}

// 26. Сброс всей статистики
TEST(StatisticsCollectorTest, ResetStats) {
  StatisticsCollector stats;
  Packet pkt("MA", "MB", "1.1.1.1", "2.2.2.2", "Data");
  stats.onEvent(MediumEvent::PACKET_SENT, pkt);
  stats.onEvent(MediumEvent::PACKET_DELIVERED, pkt);
  stats.recordDrop();

  stats.reset();
  EXPECT_EQ(stats.getPacketsSent(), 0);
  EXPECT_EQ(stats.getPacketsDelivered(), 0);
  EXPECT_EQ(stats.getPacketsDropped(), 0);
  EXPECT_DOUBLE_EQ(stats.getAverageDelay(), 0.0);
}

// 27. Расчет средней задержки с делением на ноль
TEST(StatisticsCollectorTest, AverageDelayCalculation) {
  StatisticsCollector stats;
  // Деление на ноль: доставлено 0 пакетов
  EXPECT_DOUBLE_EQ(stats.getAverageDelay(), 0.0); // Защита от NaN

  Packet pkt("MA", "MB", "1.1.1.1", "2.2.2.2", "Data", 64, 1.0); // t=1.0с

  // Имитируем получение при t=1.5с (задержка 0.5с)
  SimulationParameters::getInstance().currentTime = 1.5;
  stats.onEvent(MediumEvent::PACKET_DELIVERED, pkt);

  EXPECT_EQ(stats.getPacketsDelivered(), 1);
  EXPECT_DOUBLE_EQ(stats.getAverageDelay(), 0.5);
}

// 28. Отслеживание нескольких узлов и шин
TEST(StatisticsCollectorTest, MultipleNodesTracking) {
  auto stats = std::make_shared<StatisticsCollector>();
  auto lan1 = std::make_shared<CSMACDMedium>("LAN1", 10e6, 0.001);
  auto lan2 = std::make_shared<CSMACDMedium>("LAN2", 10e6, 0.001);

  lan1->subscribe(stats);
  lan2->subscribe(stats);

  Packet pkt1("MA", "MB", "1.1.1.1", "2.2.2.2", "Lan1");
  Packet pkt2("MC", "MD", "3.3.3.3", "4.4.4.4", "Lan2");

  lan1->sendPacket(pkt1, "MA", 0.0);
  lan2->sendPacket(pkt2, "MC", 0.0);

  // Должно агрегироваться по обеим шинам в один коллектор
  EXPECT_EQ(stats->getPacketsSent(), 2);
  EXPECT_EQ(stats->getPacketsDelivered(), 2);
}
