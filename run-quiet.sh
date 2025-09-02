#!/bin/bash

# =============================================================================
# Script de Execução Silenciosa FANET 
# Remove todos os logs verbosos, mostra apenas comunicação FANET
# =============================================================================

CONFIG=${1:-Quiet}
TIME=${2:-30s}

echo "🤫 Executando FANET em modo silencioso..."
echo "   Configuração: $CONFIG"
echo "   Tempo: $TIME"

# Limpar logs antigos primeiro
./clean-logs.sh > /dev/null 2>&1

# Compilar se necessário
if [ ! -f "./fanet-simples" ]; then
    echo "🔨 Compilando..."
    make clean && make > /dev/null 2>&1
fi

cd simulations

echo "🚁 Iniciando simulação..."
echo "─────────────────────────────────────────"

# Executar com filtros específicos para mostrar apenas FANET
../fanet-simples \
    -l /Users/rodrigo/omnetpp-workspace/inet-4.5.4/src/libINET.dylib \
    -n /Users/rodrigo/omnetpp-workspace/inet-4.5.4/src:../src:. \
    -u Cmdenv \
    -c "$CONFIG" \
    omnetpp.ini \
    --sim-time-limit="$TIME" \
    2>&1 | grep -E "(Discovery|Response|Sensor|Relay|UAV|GCS|neighbor|direct|ERROR)" | \
    grep -v "Initialized" | \
    grep -v "Setting submodule" | \
    grep -v "INFO:" | \
    sed 's/^.*FANET\.//g' | \
    sed 's/\.app\[0\]//g'

echo "─────────────────────────────────────────"
echo "✅ Simulação concluída!"
