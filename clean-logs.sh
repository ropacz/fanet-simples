#!/bin/bash

# =============================================================================
# Script de Limpeza de Logs da Simulação FANET
# =============================================================================

echo "🧹 Limpando logs da simulação FANET..."

# Remover arquivos de resultado da simulação
if [ -d "simulations/results" ]; then
    echo "📁 Limpando pasta simulations/results/"
    rm -f simulations/results/*.vci
    rm -f simulations/results/*.vec  
    rm -f simulations/results/*.sca
    rm -f simulations/results/*.anf
    echo "✅ Arquivos .vci, .vec, .sca, .anf removidos"
fi

# Remover logs de build se existirem
if [ -f "out/clang-release/fanet-simples" ]; then
    echo "🔨 Removendo executável antigo"
    rm -f out/clang-release/fanet-simples
fi

# Remover arquivos temporários
echo "🗑️ Removendo arquivos temporários"
find . -name "*.bak" -delete 2>/dev/null
find . -name "*~" -delete 2>/dev/null
find . -name ".DS_Store" -delete 2>/dev/null

# Limpar terminal (opcional)
if [ "$1" = "--clear-terminal" ]; then
    clear
fi

# Verificar se ainda há arquivos
remaining=$(find simulations/results -name "*.v*" 2>/dev/null | wc -l)
if [ $remaining -eq 0 ]; then
    echo "✅ Limpeza concluída! Todos os logs foram removidos."
else
    echo "⚠️ Ainda restam $remaining arquivos de log."
fi

echo "🚀 Pronto para nova simulação!"
