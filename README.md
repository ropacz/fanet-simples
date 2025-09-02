# FANET - Flying Ad-hoc Network Simulation

Uma implementação completa de rede ad-hoc para veículos aéreos não tripulados (UAVs) utilizando OMNeT++ 6.2.0 e INET Framework 4.5.4.

## 🎯 Visão Geral

Este projeto simula uma **Flying Ad-hoc Network (FANET)** realística com comunicação entre UAVs e uma Estação de Controle Terrestre (GCS), implementando:

- **Protocolo FANET customizado** com descoberta de vizinhos e roteamento multi-hop
- **Comunicação UDP broadcast** sobre IEEE 802.11g em modo ad-hoc  
- **Verificação rigorosa de distância** para simular limitações físicas reais
- **Mobilidade aérea realística** com ArbitraryMobility
- **Cenários de teste variados** para diferentes topologias de rede

## 🏗️ Arquitetura

### Componentes Principais

#### 🛩️ **UAVs (Aircraft)**
- Veículos aéreos com mobilidade tridimensional autônoma
- Sensores simulados (temperatura, bateria, posição GPS)
- Alcance de transmissão: **200m** (UAVs) / **300m** (GCS)
- Protocolo FANET para descoberta de vizinhos e relay de dados

#### 🏢 **GCS (Ground Control Station)**
- Estação terrestre fixa para controle e monitoramento
- Coleta dados de todos os UAVs através da rede ad-hoc
- Ponto central de coordenação com maior alcance

#### 📡 **FANETApp - Protocolo de Comunicação**
Aplicação que implementa o protocolo FANET com quatro tipos de mensagem:

1. **`NEIGHBOR_DISCOVERY`** - Descoberta periódica de vizinhos via broadcast
2. **`NEIGHBOR_RESPONSE`** - Resposta com posição e identificação do nó
3. **`SENSOR_DATA`** - Transmissão de dados de sensores (UAV → GCS)
4. **`DATA_RELAY`** - Relay multi-hop através de nós intermediários

### 🔍 Verificação de Distância

**Funcionalidade crítica implementada**: Todos os pacotes são verificados quanto à distância física antes do processamento:

```cpp
// Verificação rigorosa no socketDataArrived()
if (distance > maxTransmissionRange) {
    EV << "DROPPED: Message from " << senderAddress 
       << " out of range (" << (int)distance << "m > " 
       << (int)maxTransmissionRange << "m)" << endl;
    return; // Descarta pacote fora do alcance
}
```

Isso garante que mesmo que o IEEE 802.11 permita comunicação a longas distâncias, **apenas nós dentro do alcance especificado podem se comunicar**, simulando limitações físicas reais.

## 🚀 Como Usar

### Pré-requisitos
- OMNeT++ 6.2.0+
- INET Framework 4.5.4+
- Sistema Unix/Linux/macOS

### Instalação e Execução

```bash
# 1. Compilar o projeto
./build.sh

# 2. Executar simulação (modo gráfico)
./run.sh Default

# 3. Executar em modo silencioso
./run.sh Quiet

# 4. Executar em modo terminal
./run.sh Default Cmdenv
```

## 📋 Configurações Disponíveis

### Cenários Principais

| Configuração | Descrição | UAVs | Duração | Característica |
|-------------|-----------|------|---------|----------------|
| **Default** | Cenário completo | 5 | 5 min | Relay multi-hop natural |
| **Debug** | Teste conectividade | 1 | 1 min | Posições próximas para debug |
| **TestBasic** | Teste simples | 1 | 30s | Verificação de funcionamento |
| **Quiet** | Simulação silenciosa | 5 | 5 min | Logs mínimos, sem GUI |
| **SmallNetwork** | Rede pequena | 3 | 5 min | Menos UAVs, mais fácil de analisar |

### Cenários de Teste

| Configuração | Finalidade | Detalhes |
|-------------|------------|----------|
| **TestUDP** | Validar stack wireless | UdpBasicApp entre dois nós |
| **LargeNetwork** | Teste escalabilidade | 10 UAVs |
| **HighMobility** | UAVs rápidos | 25-35 m/s |
| **LowRange** | Alcance reduzido | 200m max |

## 📊 Funcionalidades Implementadas

### ✅ **Descoberta de Vizinhos**
- Broadcast periódico a cada 10s ± 3s (randomizado)
- Verificação automática de distância
- Timeout de vizinhos após 30s sem contato
- Logs detalhados de conectividade

### ✅ **Transmissão de Dados**  
- Dados de sensores enviados a cada 15s ± 5s
- Retry automático se GCS não responder
- Relay inteligente quando GCS fora de alcance

### ✅ **Roteamento Multi-hop**
- Algoritmo greedy: escolha do vizinho mais próximo à GCS
- Prevenção de loops com controle de hop count
- Fallback para broadcast quando relay falha

### ✅ **Mobilidade Realística**
- Movimento tridimensional com ArbitraryMobility
- Velocidade configurável (5-10 m/s padrão)
- Altitude entre 80-120m
- Área de voo restrita (200m-1800m)

## 🔧 Arquivos Principais

### Código Fonte (`src/`)
- **`FANETApp.{h,cc}`** - Protocolo FANET e lógica de comunicação
- **`Aircraft.ned`** - Definição do módulo UAV (AdhocHost)
- **`GCS.ned`** - Definição da Estação de Controle Terrestre
- **`ArbitraryMobility.{h,cc}`** - Modelo de mobilidade aérea customizado

### Simulação (`simulations/`)
- **`FANET.ned`** - Definição da topologia da rede
- **`omnetpp.ini`** - Configurações de todos os cenários
- **`environment.xml`** - Configuração do ambiente físico
- **`results/`** - Arquivos de resultado das simulações

### Scripts de Automação
- **`build.sh`** - Compilação automática
- **`run.sh`** - Execução com parâmetros
- **`clean-logs.sh`** - Limpeza de logs e resultados

## 📈 Monitoramento e Estatísticas

### Estatísticas Coletadas
- **Pacotes enviados/recebidos** por nó
- **Dados de sensores transmitidos** com sucesso  
- **Número de vizinhos** descobertos por nó
- **Transmissões wireless** no radioMedium
- **Latência de entrega** de mensagens

### Logs Organizados
```
[INFO] FANETApp: Starting neighbor discovery: UAV0 @ (1050,1050,100)
[INFO] FANETApp: Distance check: UAV0 ← GCS @ (1000,1000,10) = 71m (max: 200m)
[INFO] FANETApp: Discovery response: UAV0 → GCS (dist: 71m)
[INFO] FANETApp: Sensor data sent: UAV0 → GCS (direct)
```

## 🐛 Resolução de Problemas

### Compilação
```bash
# Se houver erro de compilação:
make clean
./build.sh
```

### Simulação não inicia
```bash
# Verificar se executável existe:
ls -la fanet-simples*

# Reconstruir se necessário:
./build.sh
```

### Zero transmissões wireless
✅ **Resolvido**: Problema era no broadcast para `192.168.1.255` no INET 4.5.4. Corrigido usando `255.255.255.255` com `limitedBroadcast = true`.

## 🔬 Detalhes Técnicos

### Stack de Protocolos
```
Aplicação:  FANETApp (protocolo customizado)
Transporte: UDP (porta 1000)  
Rede:       IPv4 (192.168.1.x/24)
Enlace:     IEEE 802.11g (modo ad-hoc)
Físico:     Ieee80211ScalarRadio (2.4 GHz)
```

### Configurações Wireless Críticas
```ini
# Modo ad-hoc funcional no INET 4.5.4
**.wlan[*].mgmt.typename = "Ieee80211MgmtAdhoc"
**.wlan[*].mgmt.ssid = "FANET" 
**.ipv4.ip.limitedBroadcast = true
**.ipv4.ip.directBroadcastInterfaces = "wlan0"
```

## 📝 Trabalhos Futuros

- [ ] Implementação de protocolo de roteamento AODV/GPSR
- [ ] Simulação de falhas de nós e recuperação da rede
- [ ] Análise de QoS e throughput da rede
- [ ] Integração com ambiente 3D mais realístico
- [ ] Simulação de interferência e obstáculos

## 📄 Licença

Este projeto é desenvolvido para fins acadêmicos e de pesquisa.

---

**Desenvolvido com OMNeT++ 6.2.0 + INET Framework 4.5.4**
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

## ⚠️ **Status do Projeto**

### ✅ **FUNCIONANDO COMPLETAMENTE**
- **Comunicação wireless**: ✅ Corrigida (22 transmissões confirmadas)
- **Descoberta de vizinhos**: ✅ Funcional com verificação de distância
- **Broadcast UDP**: ✅ Funcional com `255.255.255.255` + `limitedBroadcast=true`
- **Verificação de alcance**: ✅ Pacotes fora do `maxTransmissionRange` são descartados
- **Estatísticas**: ✅ GCS e UAVs coletando dados corretamente

### 🔧 **Correção Crítica Implementada**

**Problema Original**: Zero transmissões wireless (packets reached IP layer but never transmitted)

**Solução Aplicada**:
```cpp
// No FANETApp.cc - mudança de broadcast
socket.sendTo(packet, Ipv4Address::ALLONES_ADDRESS, destPort); // 255.255.255.255
```

```ini
# No omnetpp.ini - configurações críticas
**.ipv4.ip.limitedBroadcast = true
**.ipv4.ip.directBroadcastInterfaces = "wlan0"
```

**Resultado**: 
- Antes: `Transmission count = 0` 
- Depois: `Transmission count = 22` ✅

### 📊 **Exemplo de Execução Funcional**
```
INFO (FANETApp): Starting neighbor discovery: GCS0 @ (1000,1000,10)
INFO (FANETApp): Distance check: UAV0 ← GCS @ (1000,1000,10) = 71m (max: 200m)
INFO (FANETApp): Discovery response: UAV0 → GCS (dist: 71m)
INFO (FANETApp): FANET Stats - GCS 0: sent=6 recv=8 neighbors=1 ✅
INFO (FANETApp): FANET Stats - UAV 0: sent=6 recv=6 neighbors=1 ✅
```

---

## 📁 **Estrutura de Arquivos**

### Código Principal
```
src/
├── FANETApp.{h,cc}      # Protocolo FANET principal
├── Aircraft.ned         # Definição UAV (AdhocHost)  
├── GCS.ned             # Definição GCS (AdhocHost)
└── ArbitraryMobility.{h,cc} # Mobilidade aérea customizada
```

### Simulação
```
simulations/
├── FANET.ned           # Topologia da rede
├── omnetpp.ini         # Configurações (8 cenários)
├── environment.xml     # Ambiente físico
└── results/           # Resultados das simulações
```

### Scripts
```
./build.sh             # Compilação automática
./run.sh [config]       # Execução (Default, Debug, TestBasic...)
./clean-logs.sh         # Limpeza de logs
```

---

## 🔍 **Desenvolvimento e Depuração**

### Configurações de Teste
- **TestBasic**: Teste rápido com 1 UAV próximo à GCS
- **Debug**: Logs detalhados para troubleshooting
- **Quiet**: Simulação silenciosa para análise de dados

### Logs Estruturados
```bash
# Ver apenas comunicação FANET
./run.sh Debug Cmdenv | grep "INFO.*FANET"

# Verificar transmissões wireless
./run.sh TestBasic Cmdenv | grep "Transmission count"
```

### Métricas Disponíveis
- Pacotes enviados/recebidos por nó
- Número de vizinhos descobertos
- Dados de sensores transmitidos com sucesso
- Estatísticas do radioMedium (transmissões, recepções, etc.)

---

## 📚 **Referências Técnicas**

- **OMNeT++**: Framework de simulação de eventos discretos
- **INET Framework**: Biblioteca de protocolos de rede para OMNeT++
- **IEEE 802.11g**: Padrão wireless usado no modo ad-hoc
- **UDP**: Protocolo de transporte para comunicação rápida
- **FANET**: Flying Ad-hoc Network - extensão de MANET para UAVs

## 👨‍💻 **Autor**

Projeto desenvolvido para simulação e análise de redes FANET, implementando protocolos de comunicação ad-hoc para UAVs com verificação rigorosa de distância e roteamento multi-hop otimizado.

**Versão**: OMNeT++ 6.2.0 + INET Framework 4.5.4  
**Status**: ✅ Completamente funcional  
**Última atualização**: Setembro 2025
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
