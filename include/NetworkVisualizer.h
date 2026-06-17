#pragma once
#include "IObserver.h"
#include <string>

/**
 * NetworkVisualizer — модуль визуализации (паттерн Observer / Subscriber).
 *
 * Второй подписчик на события CSMACDMedium (помимо StatisticsCollector).
 * Отображает события среды в виде ASCII-диаграммы в консоли, позволяя
 * наблюдать за состоянием сети в реальном времени.
 *
 * Паттерн Observer: подписывается через CSMACDMedium::subscribe().
 * CSMACDMedium сам вызывает onEvent() при каждом событии.
 *
 * Пример вывода:
 *   [VIZ] ──→ пакет #3  [192.168.1.2 → 192.168.2.2]  ОТПРАВКА
 *   [VIZ] ✓   пакет #3  [192.168.1.2 → 192.168.2.2]  ДОСТАВЛЕН
 *   [VIZ] ✗✗  пакет #5  [192.168.1.2 → 192.168.2.2]  КОЛЛИЗИЯ!
 */
class NetworkVisualizer : public IObserver {
public:
    /**
     * @param mediumName — имя среды (для пометки в выводе)
     * @param verbose    — если false, коллизии не выводятся (тихий режим)
     */
    explicit NetworkVisualizer(const std::string& mediumName,
                               bool               verbose = true);

    // Реализация интерфейса IObserver
    void onEvent(MediumEvent event, const Packet& pkt) override;

    // Получить количество событий, которое обработал визуализатор
    int getEventsHandled() const { return eventsHandled_; }

    // Включить/выключить подробный вывод
    void setVerbose(bool verbose) { verbose_ = verbose; }

private:
    std::string mediumName_;   // Имя наблюдаемой среды передачи
    bool        verbose_;      // Режим подробного вывода
    int         eventsHandled_ = 0; // Счётчик обработанных событий
};
