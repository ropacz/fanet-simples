# FANET - Flying Ad-hoc Network Simulation

Simulação de uma Rede Ad-hoc Voadora (FANET) utilizando OMNeT++ e INET Framework.

## Visão Geral

Este projeto implementa uma simulação completa de rede ad-hoc para comunicação entre Veículos Aéreos Não Tripulados (UAVs) e uma Estação de Controle Terrestre (GCS). A implementação inclui:

- **Protocolo FANET** com descoberta de vizinhos e roteamento multi-hop
- **Comunicação UDP** sobre IEEE 802.11 em modo ad-hoc
- **Mobilidade realística** dos UAVs com ArbitraryMobility
- **Cenários de relay forçado** para demonstrar roteamento multi-hop
- **Logs organizados** para análise de comportamento da rede

## Arquitetura

### Componentes Principais

1. **UAVs (Aircraft)**: 
   - Veículos aéreos com mobilidade autônoma
   - Sensores simulados (temperatura, pressão, GPS)
   - Protocolo FANET para comunicação
   - Capacidade de relay multi-hop

2. **GCS (Ground Control Station)**: 
   - Estação terrestre fixa para controle e monitoramento
   - Coleta dados de todos os UAVs da rede
   - Maior alcance de transmissão (500m vs 300m dos UAVs)

3. **FANETApp**: 
   - Aplicação de comunicação implementando o protocolo FANET
   - Gerenciamento de vizinhos e roteamento
   - Logs otimizados para análise de rede

### Protocolo FANET

O protocolo implementa quatro tipos de mensagens otimizadas:

- `NEIGHBOR_DISCOVERY`: Descoberta de vizinhos via broadcast periódico
- `NEIGHBOR_RESPONSE`: Resposta à descoberta com posição e tipo de nó
- `SENSOR_DATA`: Transmissão de dados de sensores dos UAVs para GCS
- `DATA_RELAY`: Relay de dados através de UAVs intermediários (multi-hop)

### Características Especiais

#### Cenário de Relay Natural
- **UAV[3]** posicionado a ~778m do GCS (fora do alcance de 450m)
- **UAV[4]** posicionado a ~1061m do GCS (bem fora do alcance de 450m)
- Topologia em cadeia: UAV[4] → UAV[3] → UAV[1] → GCS
- Demonstra roteamento multi-hop baseado em limitações físicas reais
- Simula condições operacionais onde nem todos os UAVs têm linha de visada para GCS

## Estrutura do Projeto

```
fanet-simples/
├── src/                    # Código fonte organizado
│   ├── FANETApp.cc        # Aplicação FANET (546 linhas organizadas)
│   ├── FANETApp.h         # Cabeçalho com estruturas otimizadas
│   ├── FANETApp.ned       # Definição da aplicação
│   ├── Aircraft.cc        # Implementação do UAV
│   ├── Aircraft.h         # Cabeçalho do UAV
│   ├── Aircraft.ned       # Módulo UAV com IEEE 802.11 ad-hoc
│   ├── GCS.ned           # Módulo GCS com configuração de rede
│   └── ArbitraryMobility.*# Modelo de mobilidade 3D customizado
├── simulations/           # Configurações de simulação
│   ├── FANET.ned         # Topologia da rede FANET
│   ├── omnetpp.ini       # Parâmetros organizados por seções
│   ├── environment.xml   # Configuração do ambiente 3D
│   └── results/          # Resultados e métricas das simulações
├── build.sh              # Script de compilação
├── run.sh                # Script de execução
└── README.md             # Documentação completa
```

## Implementação Detalhada

### FANETApp.cc - Estrutura Organizada

O código foi completamente reorganizado em seções funcionais:

1. **Inicialização e Configuração** (linhas 1-150)
   - Configuração de soquetes UDP
   - Inicialização de timers
   - Parâmetros de rede

2. **Gerenciamento de Mensagens** (linhas 151-250)
   - Processamento de timers
   - Roteamento de mensagens
   - Controle de fluxo

3. **Protocolo de Descoberta** (linhas 251-350)
   - Neighbor Discovery broadcast
   - Processamento de respostas
   - Atualização de tabela de vizinhos

4. **Transmissão de Dados** (linhas 351-450)
   - Coleta de dados de sensores
   - Envio direto ou via relay
   - Implementação de relay forçado

5. **Roteamento Multi-hop** (linhas 451-546)
   - Seleção de relay ótimo
   - Processamento de dados relay
   - Prevenção de loops

## Melhorias Implementadas

### Organização do Código
- **Refatoração completa** do FANETApp.cc (691 → 546 linhas)
- **Estrutura modular** com responsabilidades bem definidas
- **Eliminação de duplicações** e funções redundantes
- **Comentários estratégicos** apenas em lógica complexa

### Otimização de Logs
- **Logs concisos** substituindo saídas verbosas
- **Informações essenciais** preservadas para depuração
- **Formato consistente** para análise automatizada
- **Redução de ruído** nos logs de baixo nível

### Funcionalidades Avançadas
- **Relay natural** baseado em limitações físicas de alcance
- **Validação de entrada** em todos os processadores de mensagem
- **Gerenciamento robusto** de recursos e timers
- **Tratamento de erros** em cenários de rede adversos

## Desenvolvimento e Contribuição

### Arquitetura do Código
O projeto segue as melhores práticas de desenvolvimento:

- **Separação de responsabilidades** clara entre módulos
- **Uso eficiente de memória** com gerenciamento adequado
- **Compatibilidade** com INET 4.5.4 e OMNeT++ 6.2.0
- **Escalabilidade** para diferentes tamanhos de rede

### Padrões de Codificação
- Nomes significativos para variáveis e funções
- Gerenciamento correto de ponteiros e alocação
- Tratamento defensivo de inputs
- Otimização apenas em seções críticas

### Depuração e Teste
- Logs estruturados para análise de comportamento
- Validação de estado da rede em tempo real
- Métricas de desempenho integradas
- Cenários de teste específicos para cada funcionalidade

---

## Autor
Projeto desenvolvido para simulação e análise de redes FANET utilizando OMNeT++ e INET Framework, implementando protocolos de comunicação ad-hoc para UAVs com roteamento multi-hop otimizado.
   - Implementação de relay forçado

5. **Roteamento Multi-hop** (linhas 451-546)
   - Seleção de relay ótimo
   - Processamento de dados relay
   - Prevenção de loops

## Fluxo de Comunicação

### 1. Descoberta de Vizinhos (Neighbor Discovery)
- UAVs e GCS enviam broadcasts periódicos (a cada 2 segundos)
- Respostas contêm posição geográfica (x, y, z) e tipo de nó (UAV/GCS)
- Distância calculada em 3D para determinar alcance efetivo
- **UAV[4]** posicionado fora do alcance direto do GCS

### 2. Transmissão de Dados de Sensores
- UAVs coletam dados simulados: temperatura (15-35°C), pressão (900-1100 hPa)
- Coordenadas GPS obtidas do modelo de mobilidade
- Tentativa de envio direto para GCS quando disponível
- **UAV[4]** usa relay naturalmente por estar fora do alcance

### 3. Roteamento Multi-hop (Relay)
- Quando GCS não está no alcance direto, busca UAV intermediário
- Seleção do relay baseada na menor distância para GCS
- Contador de hops previne loops infinitos (máximo 5 hops)
- Logs otimizados mostram caminho completo dos dados

### 4. Conectividade e Monitoramento
- Verificação periódica de conectividade (a cada 10 segundos)
- Estatísticas de pacotes enviados/recebidos por nó
- Métricas de alcance e qualidade do link

## Configuração Técnica

### Requisitos do Sistema
- **OMNeT++**: 6.2.0 ou superior
- **INET Framework**: 4.5.4 ou superior
- **Compilador**: g++ com suporte C++14
- **Sistema**: Linux, macOS ou Windows

### Parâmetros de Rede

#### Configuração Wireless
- **Protocolo**: IEEE 802.11g OFDM
- **Frequência**: 2.412 GHz (Canal 1)
- **Modo**: Ad-hoc (IBSS)
- **Potência de transmissão**: 20 mW
- **Alcance efetivo**: 
  - UAVs: ~300m
  - GCS: ~500m (maior potência)

#### Configuração de Mobilidade
- **Área de simulação**: 2000x2000m
- **Altitude dos UAVs**: 50-200m (varia dinamicamente)
- **Velocidade**: 15-25 m/s
- **Padrão de movimento**: ArbitraryMobility com trajetórias realísticas

### Cenários Implementados

#### 1. Cenário Padrão (Default)
- **Nós**: 5 UAVs + 1 GCS
- **Comportamento**: Comunicação normal com relay automático
- **Objetivo**: Demonstrar funcionamento básico da FANET

#### 2. Cenário de Relay Forçado
- **UAV[4]** configurado para usar apenas relay
- **Implementação**: 
  - Bloqueia descoberta direta com GCS
  - Força roteamento através de UAVs intermediários
  - Demonstra robustez do protocolo multi-hop
- **Logs**: Mensagens específicas indicam uso de relay forçado

### Execução e Compilação

```bash
# Compilar o projeto
make clean && make

# Executar simulação com interface gráfica
./run.sh Default Qtenv

# Executar simulação em linha de comando
./run.sh Default Cmdenv

# Executar com filtros de log para análise
./run.sh Default Cmdenv 2>&1 | grep -E "(Discovery|Response|Sensor|Relay)"

# Limpar logs e arquivos de resultado
./clean-logs.sh

# Script de compilação automatizada
./build.sh
```

### Limpeza de Logs

Para limpar todos os logs e resultados da simulação:

```bash
# Usar o script de limpeza (recomendado)
./clean-logs.sh

# Ou manualmente:
rm -f simulations/results/*.vci simulations/results/*.vec simulations/results/*.sca
```

O script `clean-logs.sh` remove:
- Arquivos `.vci` (configuração da simulação)
- Arquivos `.vec` (dados vetoriais de resultados) 
- Arquivos `.sca` (dados escalares de estatísticas)
- Executáveis compilados antigos
- Arquivos temporários (`.bak`, `*~`, `.DS_Store`)

### Análise de Logs

#### Logs Otimizados
A implementação atual utiliza logs concisos e informativos:

```
// Descoberta de vizinhos
Discovery sent by UAV0 @ (1052.87, 1037.14, 108.608)
New neighbor: UAV 192.168.1.3 @ 127m

// Transmissão de dados
Sensor data sent to GCS: T=25.3°C P=1013hPa
Data relayed via UAV2 to GCS (3 hops)

// Cenário de relay natural
Sensor data relayed: UAV4 → UAV2 → GCS (no direct path)
Discovery ignored: 192.168.1.1 out of range (650m > 300m)
```

#### Filtros Úteis para Depuração
```bash
# Ver apenas mensagens da aplicação FANET
grep -E "(Discovery|Response|Sensor|Relay)" simulation.log

# Monitorar UAV específico
grep "uav\[4\]" simulation.log

# Verificar conectividade
grep -E "(New neighbor|lost|blocked)" simulation.log
```

## Métricas e Resultados

### Dados Coletados Automaticamente
- **Pacotes enviados/recebidos** por cada nó
- **Dados de sensores transmitidos** com timestamps
- **Número de vizinhos descobertos** por UAV
- **Distâncias e conectividade** da rede em tempo real
- **Eficiência do roteamento** multi-hop

### Arquivos de Resultado
- `Default-#0.vec`: Dados vetoriais para gráficos
- `Default-#0.vci`: Índices dos vetores
- Logs de simulação com detalhes de comunicação

### Análise de Desempenho
- **Taxa de entrega**: Percentual de pacotes que chegam ao GCS
- **Latência**: Tempo de transmissão direto vs relay
- **Overhead de roteamento**: Mensagens de controle vs dados
- **Cobertura da rede**: Alcance efetivo dos UAVs

## Cenários de Teste Avançados

### 1. Cenário Padrão (Default)
- **Configuração**: 5 UAVs + 1 GCS
- **Comportamento**: Comunicação híbrida (direta + relay)
- **Objetivo**: Validar funcionamento completo do protocolo

#### 2. Cenário de Relay Natural
- **UAV[4]**: Fora do alcance do GCS por distância física
- **Outros UAVs**: Comunicação direta com GCS
- **Demonstração**: Roteamento multi-hop baseado em limitações reais

### 3. Cenário de Alta Mobilidade
- **Velocidade aumentada**: 30-40 m/s
- **Teste**: Estabilidade da rede com topologia dinâmica

### 4. Cenário de Alcance Limitado
- **Potência reduzida**: Simula condições adversas
- **Foco**: Eficiência do algoritmo de relay

### 5. Cenário de Rede Densa
- **10+ UAVs**: Teste de escalabilidade
- **Análise**: Overhead de descoberta de vizinhos
