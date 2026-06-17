#!/bin/bash
set -e
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' 
run_tests() {
    echo -e "${GREEN}[1/2] Unit-тесты кор-классов...${NC}"
    ./unit_tests
    echo -e "${GREEN}✓ Unit-тесты успешно пройдены!${NC}"
    echo -e "${GREEN}[2/2] Интеграционные сценарии...${NC}"
    scenarios=(
        scenario1_simple_transfer
        scenario2_collision
        scenario3_routing
        scenario4_heavy_load
        scenario5_mixed_traffic
        scenario6_network_failure
    )

    failed=0
    for scenario in "${scenarios[@]}"; do
        echo -e "${YELLOW}---------------------------------------------------------${NC}"
        echo -e "${YELLOW}Запуск: ./${scenario}${NC}"
        if ./"$scenario"; then
            echo -e "${GREEN}✓ ${scenario} — OK${NC}"
        else
            echo -e "${RED}✗ ${scenario} — FAILED${NC}"
            failed=$((failed + 1))
        fi
        echo ""
    done
    if [ "$failed" -eq 0 ]; then
        echo -e "${GREEN}✓ Все 34 теста пройдены успешно${NC}"
        exit 0
    else
        echo -e "${RED}✗ Упало сценариев: ${failed}${NC}"
        exit 1
    fi
}

if [ "$1" = "test" ]; then
    shift
    run_tests
fi

if [ $# -gt 0 ]; then
    echo -e "${CYAN}[Docker] Запуск: $@${NC}"
    exec "$@"
fi

echo -e "${CYAN}[Docker] Запуск симулятора (основное приложение)${NC}"
exec ./network_simulator "$@"
