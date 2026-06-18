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

    std::shared_ptr<Host> addHost(
        const std::string& name,
        const std::string& mac,
        const std::string& ip);


    std::shared_ptr<Router> addRouter(
        const std::string& name,
        const std::string& mac,
        const std::string& ip);


    std::shared_ptr<CSMACDMedium> addMedium(
        const std::string& name,
        double bandwidth   = 10e6,
        double propagDelay = 0.001);


    void connectNodeToMedium(std::shared_ptr<Node>         node,
                             std::shared_ptr<CSMACDMedium> medium);

    void connectRouterInterface(std::shared_ptr<Router>       router,
                                int                           interfaceId,
                                std::shared_ptr<CSMACDMedium> medium);

    void startSimulation(double durationSeconds);
    void stopSimulation();
    void printStats() const;
    std::shared_ptr<StatisticsCollector> getStats() const { return stats_; }

private:
    std::vector<std::shared_ptr<Host>>         hosts_;
    std::vector<std::shared_ptr<Router>>       routers_;
    std::vector<std::shared_ptr<CSMACDMedium>> media_;
    std::shared_ptr<StatisticsCollector> stats_;
    bool running_ = false;
    double currentTime_ = 0.0;
};
