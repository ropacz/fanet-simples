#!/bin/bash
# Build script for FANET simulation

echo "🚀 Compilando FANET..."

# Limpeza de builds anteriores
make clean

# Geração do Makefile
opp_makemake -f --deep -KINET_PROJ=/Users/rodrigo/omnetpp-workspace/inet-4.5.4 -DINET_IMPORT -I/Users/rodrigo/omnetpp-workspace/inet-4.5.4/src -L/Users/rodrigo/omnetpp-workspace/inet-4.5.4/src -lINET

# Compilação
make MODE=release -j$(sysctl -n hw.ncpu)

if [ $? -eq 0 ]; then
    echo "✅ Build concluído com sucesso!"
    echo "Execute: ./run.sh para rodar a simulação"
else
    echo "❌ Erro no build"
    exit 1
fi
