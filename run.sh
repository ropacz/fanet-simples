#!/bin/bash
# FANET Simulation Runner
# Usage: ./run.sh [config] [ui_mode]
# Configs: Default, Quiet, SmallNetwork  
# UI Modes: Qtenv (gráfico), Cmdenv (terminal)

CONFIG=${1:-Default}
UI_MODE=${2:-Qtenv}

echo "🚁 Executando simulação FANET..."
echo "   Configuração: $CONFIG"
echo "   Interface: $UI_MODE"

# Verificar se o executável existe
if [ ! -f "./fanet-simples" ]; then
    echo "❌ Executável não encontrado. Execute ./build.sh primeiro"
    exit 1
fi

# Navegar para o diretório de simulações
cd simulations

# Executar simulação com caminho NED correto
if [ "$UI_MODE" = "Cmdenv" ]; then
    if [ "$CONFIG" = "Quiet" ]; then
        echo "🔇 Executando em modo silencioso..."
        # Modo silencioso com filtros
        ../fanet-simples -l /Users/rodrigo/omnetpp-workspace/inet-4.5.4/src/libINET.dylib -n /Users/rodrigo/omnetpp-workspace/inet-4.5.4/src:../src:. -u "$UI_MODE" -c "$CONFIG" omnetpp.ini --sim-time-limit=30s 2>&1 | grep -E "(Discovery|Response|Sensor|Relay|ERROR|WARNING)"
    else
        # Modo cmdenv com limite de tempo para teste
        ../fanet-simples -l /Users/rodrigo/omnetpp-workspace/inet-4.5.4/src/libINET.dylib -n /Users/rodrigo/omnetpp-workspace/inet-4.5.4/src:../src:. -u "$UI_MODE" -c "$CONFIG" omnetpp.ini --sim-time-limit=30s
    fi
else
    # Modo gráfico normal
    ../fanet-simples -l /Users/rodrigo/omnetpp-workspace/inet-4.5.4/src/libINET.dylib -n /Users/rodrigo/omnetpp-workspace/inet-4.5.4/src:../src:. -u "$UI_MODE" -c "$CONFIG" omnetpp.ini
fi
