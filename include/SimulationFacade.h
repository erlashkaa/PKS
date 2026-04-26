#pragma once
#include "Node.h"
#include "Host.h"
#include "Router.h"
#include "CSMACDMedium.h"
#include "StatisticsCollector.h"
#include <vector>
#include <memory>
#include <string>

/**
 * SimulationFacade — главный класс симулятора (паттерн Facade).
 *
 * Предоставляет простой, высокоуровневый интерфейс:
 *   addHost(), addRouter(), addMedium(), connectNodeToMedium()
 *   startSimulation(), stopSimulation(), printStats()
 *
 * Скрывает под капотом:
 *   - управление тиками времени
 *   - регистрацию наблюдателей
 *   - обход всех узлов и сред на каждом шаге
 *
 * Топология сети:
 *   Host-A ──┐
 *            ├── [Medium-1] ── Router ── [Medium-2] ── Host-B
 *   ...     ─┘
 */
class SimulationFacade {
public:
    SimulationFacade();

    // ---- Построение топологии ----

    /**
     * Добавить хост в симуляцию.
     * @return shared_ptr на созданный Host (для дальнейшей настройки)
     */
    std::shared_ptr<Host> addHost(
        const std::string& name,
        const std::string& mac,
        const std::string& ip);

    /**
     * Добавить маршрутизатор в симуляцию.
     * @return shared_ptr на созданный Router
     */
    std::shared_ptr<Router> addRouter(
        const std::string& name,
        const std::string& mac,
        const std::string& ip);

    /**
     * Создать среду передачи (сегмент сети).
     * @return shared_ptr на созданный CSMACDMedium
     */
    std::shared_ptr<CSMACDMedium> addMedium(
        const std::string& name,
        double bandwidth   = 10e6,
        double propagDelay = 0.001);

    /**
     * Подключить узел к среде (для Host и Router-интерфейса 0).
     * Узел начнёт «слышать» среду и сможет в неё отправлять пакеты.
     */
    void connectNodeToMedium(std::shared_ptr<Node>         node,
                             std::shared_ptr<CSMACDMedium> medium);

    /**
     * Подключить интерфейс маршрутизатора с заданным номером к среде.
     * Используется для роутеров, у которых несколько интерфейсов.
     */
    void connectRouterInterface(std::shared_ptr<Router>       router,
                                int                           interfaceId,
                                std::shared_ptr<CSMACDMedium> medium);

    // ---- Управление симуляцией ----

    /**
     * Запустить симуляцию на заданное количество секунд.
     * Внутри: цикл тиков с шагом tickStep из SimulationParameters.
     * На каждом тике: обновляются среды, затем вызываются tick() у всех хостов.
     */
    void startSimulation(double durationSeconds);

    // Остановить симуляцию досрочно (устанавливает флаг)
    void stopSimulation();

    // Вывести итоговую статистику
    void printStats() const;

    // Получить сборщик статистики (для внешнего использования)
    std::shared_ptr<StatisticsCollector> getStats() const { return stats_; }

private:
    // Все хосты в симуляции
    std::vector<std::shared_ptr<Host>>         hosts_;
    // Все роутеры в симуляции
    std::vector<std::shared_ptr<Router>>       routers_;
    // Все среды передачи в симуляции
    std::vector<std::shared_ptr<CSMACDMedium>> media_;

    // Единственный сборщик статистики (подписан на все среды)
    std::shared_ptr<StatisticsCollector> stats_;

    // Флаг остановки (выставляется в true при вызове stopSimulation())
    bool running_ = false;

    // Текущее модельное время (секунды)
    double currentTime_ = 0.0;
};
