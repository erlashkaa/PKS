# Симулятор компьютерной сети (CSMA/CD)

Учебный проект по дисциплине «Проектирование компьютерных сетей».  
Симулятор моделирует работу сети Ethernet с протоколом доступа к среде **CSMA/CD**
и демонстрирует применение четырёх паттернов проектирования (GoF).

---

## Паттерны проектирования

| Паттерн | Класс(ы) | Назначение |
|---|---|---|
| **Singleton** | `SimulationConfig` | Единый объект глобальных параметров (шаг тика, backoff, и т.д.) |
| **Factory Method** | `NodeFactory` | Централизованное создание `Host` и `Router` по строке-типу |
| **Strategy** | `ITrafficGenerationStrategy` → `ConstantBitRateStrategy`, `PoissonStrategy`, `BurstyStrategy` | Взаимозаменяемые алгоритмы генерации трафика в `Host` |
| **Observer** | `CSMACDMedium` (Publisher) → `StatisticsCollector`, `NetworkVisualizer` (Subscribers) | Уведомление подписчиков о событиях среды (отправка, доставка, коллизия) |

---

## Структура директорий

```
project/
├── CMakeLists.txt           # Система сборки (CMake ≥ 3.10, C++17)
├── Dockerfile               # Многостадийная сборка для Linux
├── docker-compose.yml       # Запуск симулятора и тестов в контейнере
├── docker-entrypoint.sh     # Точка входа контейнера
├── README.md                # Этот файл
├── include/                 # Публичные заголовочные файлы (.h)
│   ├── ITrafficGenerationStrategy.h   # Интерфейс Strategy
│   ├── IObserver.h                    # Интерфейс Observer + enum MediumEvent
│   ├── Packet.h                       # Структура пакета данных
│   ├── Node.h                         # Абстрактный базовый класс узла
│   ├── Host.h                         # Конечный узел сети (использует Strategy)
│   ├── Router.h                       # Маршрутизатор (таблица маршрутов)
│   ├── CSMACDMedium.h                 # Среда передачи CSMA/CD (Publisher)
│   ├── TrafficStrategies.h            # CBR и Bursty стратегии
│   ├── PoissonStrategy.h              # Пуассоновская стратегия (новая)
│   ├── StatisticsCollector.h          # Сборщик статистики (Observer)
│   ├── NetworkVisualizer.h            # Визуализатор событий (Observer)
│   ├── NodeFactory.h                  # Фабрика узлов (Factory Method)
│   ├── SimulationFacade.h             # Фасад симулятора
│   ├── SimulationConfig.h             # Singleton конфигурации
│   └── SimulationParameters.h        # Устаревший алиас → SimulationConfig
└── src/                     # Исходные файлы (.cpp)
    ├── main.cpp
    ├── Packet.cpp
    ├── Node.cpp
    ├── Host.cpp
    ├── Router.cpp
    ├── CSMACDMedium.cpp
    ├── TrafficStrategies.cpp
    ├── PoissonStrategy.cpp
    ├── NetworkVisualizer.cpp
    ├── StatisticsCollector.cpp
    ├── NodeFactory.cpp
    └── SimulationFacade.cpp
```

---

## Используемые библиотеки

| Библиотека | Тип | Назначение |
|---|---|---|
| C++ STL | Встроенная | `vector`, `memory`, `queue`, `optional`, `random` |
| `<random>` (C++11) | Встроенная | `std::mt19937`, `std::exponential_distribution` для Poisson |
| Windows API (`windows.h`) | Системная | Настройка UTF-8 консоли (`SetConsoleOutputCP`) |

Сторонних библиотек нет — только стандартная библиотека C++17.

---

## Требования

- **CMake** ≥ 3.10
- **Компилятор** с поддержкой C++17:
  - MSVC 2019/2022 (Windows)
  - GCC ≥ 7 / Clang ≥ 5 (Linux/macOS)
- **ОС**: Windows (используется `windows.h` для UTF-8 консоли)

---

## Сборка и запуск

> Команды выполняются из **корня проекта** (папка с `CMakeLists.txt`).  
> Исполняемый файл создаётся только в `build/`, не в корне проекта.

### Что должно быть установлено (Windows)

| Инструмент | Проверка | Установка |
|---|---|---|
| CMake ≥ 3.10 | `cmake --version` | `winget install Kitware.CMake` |
| Ninja | `ninja --version` | `winget install Ninja-build.Ninja` |
| MinGW g++ | `g++ --version` | [winlibs.com](https://winlibs.com/) или MSYS2 |

После установки **полностью закройте PowerShell и откройте снова** (или перезапустите Cursor).

**Если пишет «cmake не распознано»** — перезапустите терминал/Cursor или вызывайте CMake по полному пути:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -S . -B build -G Ninja
& "C:\Program Files\CMake\bin\cmake.exe" --build build
.\build\network_simulator.exe
```

---

### Сборка и запуск (CMake)

```powershell
cd "C:\Users\gabdu\OneDrive\Рабочий стол\вуз\пкс\project"

cmake -S . -B build -G Ninja
cmake --build build
.\build\network_simulator.exe
```

Если раньше была ошибка про `nmake`, удалите папку `build` и повторите.

С аргументами:

```powershell
.\build\network_simulator.exe --duration 3
.\build\network_simulator.exe --help
```

### Тесты

```powershell
cmake -S . -B build
cmake --build build
cd build
ctest --output-on-failure
```

Подробнее — в [TESTING.md](TESTING.md).

---

### Если `cmake -S . -B build` выдаёт ошибку про `nmake`

CMake выбрал генератор Visual Studio (NMake), а он у вас не установлен. Исправление:

```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build -G Ninja
cmake --build build
```

Или явно указать Ninja:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
```

---

### Аргументы командной строки

| Аргумент | Описание |
|---|---|
| `--duration 3` / `-d 3` | Длительность симуляции в секундах (по умолчанию 5) |
| `--help` / `-h` | Справка |

---

## Docker

Проект собирается и тестируется в чистом Linux-окружении (Ubuntu 22.04) через **многостадийный Dockerfile**:
на этапе сборки выполняются `cmake`, `ctest`, в финальный образ попадают только готовые бинарники.

**Требования:** установленный [Docker Desktop](https://www.docker.com/products/docker-desktop/) (Windows) или Docker Engine (Linux).

### Сборка образа

```powershell
# PowerShell
cd "путь\к\project"
docker build -t network-simulator .
```

```bash
# Linux / macOS
cd project/
docker build -t network-simulator .
```

> Если команда `docker build .` выдаёт `Dockerfile: no such file or directory`, значит вы **не в корне проекта**. Убедитесь, что в текущей папке есть файл `Dockerfile`.

### Запуск симулятора в контейнере

```bash
# Основное приложение (симулятор, 5 секунд по умолчанию)
docker run --rm network-simulator

# С аргументами командной строки
docker run --rm network-simulator ./network_simulator --duration 2
docker run --rm network-simulator ./network_simulator --help
```

### Запуск тестов внутри контейнера

Тесты (GoogleTest + 6 интеграционных сценариев, всего 34) также прогоняются автоматически **на этапе сборки образа** (`RUN ctest`). Дополнительно их можно запустить в уже собранном контейнере:

```bash
# Вся тестовая суита
docker run --rm network-simulator test

# Отдельный unit-тест или сценарий
docker run --rm network-simulator ./unit_tests
docker run --rm network-simulator ./scenario3_routing
```

### Docker Compose

```bash
cd project/

# Симулятор (5 секунд)
docker compose run --rm simulator

# Все тесты
docker compose run --rm simulator-tests
```

| Команда | Действие |
|---|---|
| `docker build -t network-simulator .` | Сборка образа + `ctest` на этапе build |
| `docker run --rm network-simulator` | Запуск симулятора |
| `docker run --rm network-simulator test` | Прогон всех тестов в контейнере |
| `docker compose run --rm simulator` | Симулятор через compose |
| `docker compose run --rm simulator-tests` | Тесты через compose |

---

## Пример вывода

```
╔══════════════════════════════════════════════╗
║   СИМУЛЯТОР КОМПЬЮТЕРНОЙ СЕТИ (CSMA/CD)      ║
║   Паттерны: Singleton | Factory | Strategy   ║
║             Observer                         ║
╚══════════════════════════════════════════════╝

[Singleton: SimulationConfig]
  tickStep      = 1 мс
  backoffSlot   = 5 мс
  maxRetries    = 10

[Factory Method: NodeFactory]
[Топология] Добавлен Host: Host-A | MAC=AA:BB:CC:DD:EE:01 | IP=192.168.1.2
[Топология] Добавлен Host: Host-B | MAC=AA:BB:CC:DD:EE:02 | IP=192.168.2.2
[Топология] Добавлен Host: Host-C | MAC=AA:BB:CC:DD:EE:03 | IP=192.168.1.3
[Топология] Добавлен Router: Router-1 | MAC=AA:BB:CC:DD:EE:FF | IP=192.168.1.1

...

════════════════════════════════════════════
  СИМУЛЯЦИЯ ЗАПУЩЕНА
  Длительность : 5 с
  Шаг тика     : 1 мс
  Всего тиков  : 5000
════════════════════════════════════════════

[t=0.000] Host-A → пытается отправить пакет #1 [192.168.1.2 → 192.168.2.2]
[VIZ:LAN2] ──→  пакет #1  [192.168.1.2 → 192.168.2.2]  ПЕРЕДАЧА
[VIZ:LAN2]  ✓   пакет #1  [192.168.1.2 → 192.168.2.2]  ДОСТАВЛЕН

...

╔══════════════════════════════════════════╗
║       СТАТИСТИКА СИМУЛЯЦИИ               ║
╠══════════════════════════════════════════╣
║  Отправлено пакетов  :            XX     ║
║  Доставлено пакетов  :            XX     ║
║  Коллизий            :             X     ║
║  Успешность доставки :          XX.X%    ║
╚══════════════════════════════════════════╝
```

---

## Архитектурная схема

```
             ┌─────────────────────────────────────────┐
             │           SimulationFacade              │
             │  (Facade — скрывает детали симуляции)   │
             └───────┬─────────────┬───────────────────┘
                     │             │
          ┌──────────▼───┐   ┌─────▼──────────────────────────┐
          │  NodeFactory │   │      SimulationConfig          │
          │(Factory Meth)│   │        (Singleton)             │
          └──────┬───────┘   └────────────────────────────────┘
                 │
     ┌───────────┼───────────┐
     ▼           ▼           ▼
  ┌──────┐   ┌──────┐   ┌────────┐
  │ Host │   │ Host │   │ Router │
  │  A   │   │  C   │   │   1    │
  └──┬───┘   └──┬───┘   └──┬─┬──┘
     │  Strategy│           │ │
  ┌──┴──┐  ┌───┴──┐        │ │
  │ CBR │  │Poison│        │ │
  └─────┘  └──────┘        │ │
                            │ │
     ┌──────────────────────┘ │
     ▼                        ▼
  [LAN1]                   [LAN2]──── Host-B ──── BurstyStrategy
  CSMACDMedium             CSMACDMedium
  (Publisher)              (Publisher)
     │                        │
     ├── StatisticsCollector  ├── StatisticsCollector
     └── NetworkVisualizer    └── NetworkVisualizer
         (Observer/Subscriber)    (Observer/Subscriber)
```
