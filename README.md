# FANET - Flying Ad-hoc Network Simulation

Simulação de rede ad-hoc para drones (UAVs) usando OMNeT++ 6.2.0 e INET 4.5.4.

## 🎯 Visão Geral

Protocolo FANET com **algoritmo de mesh routing** para comunicação multi-hop entre UAVs e Ground Control Station (GCS).

### Características Principais
- **Mesh routing ad-hoc** com descoberta automática de rotas
- **Multi-hop communication** através de UAVs intermediários 
- **Verificação de distância física** para realismo
- **Finalização automática** (evita loops infinitos)
- **Área de simulação**: 2000x2000m

## 🚀 Como Usar

### Compilação e Execução
```bash
# Compilar
make clean && make

# Executar configurações
./run.sh -c Default      # 2 UAVs, 45s (padrão)
./run.sh -c Quick        # 2 UAVs, 30s (teste relay)
./run.sh -c Production   # 5 UAVs, 120s (rede completa)
./run.sh -c FastTest     # 1 UAV, 20s (teste rápido)
```

## 📊 Resultados Típicos

### Configuration Production (5 UAVs)
```
GCS: sent=7 recv=28 data_recv=10 neighbors=3
UAV[0]: sent=4 recv=30 data_sent=2 neighbors=1
UAV[1]: sent=6 recv=33 data_sent=2 neighbors=1  
UAV[2]: sent=6 recv=31 data_sent=2 neighbors=1
UAV[3]: sent=3 recv=28 data_sent=2 neighbors=0
UAV[4]: sent=4 recv=30 data_sent=2 neighbors=1
```

**✅ Sucessos:**
- GCS recebeu 10 pacotes de dados de múltiplos UAVs
- Todos os UAVs conseguiram enviar dados (mesh routing funcionando)
- Rede mesh conectou UAVs distantes através de relays

## ⚙️ Configurações

### Cenários Disponíveis

| Config | UAVs | Duração | Propósito |
|--------|------|---------|-----------|
| `Default` | 2 | 45s | Teste básico |
| `Quick` | 2 | 30s | Teste relay |
| `Production` | 5 | 120s | Rede completa |
| `FastTest` | 1 | 20s | Teste rápido |

### Parâmetros Principais
- **Alcance UAV**: 500m
- **Alcance GCS**: 600m  
- **Velocidade**: 10-15 m/s
- **Área**: 200m-1800m
- **Altitude**: 95-105m

## 📁 Estrutura

```
src/
├── FANETApp.cc/h     # Protocolo FANET com mesh routing
├── Aircraft.cc/h/ned # Modelo de UAV
├── ArbitraryMobility.cc/h/ned # Mobilidade aérea
└── GCS.ned           # Ground Control Station

simulations/
├── omnetpp.ini       # Configurações de simulação
├── FANET.ned         # Topologia da rede
└── results/          # Resultados das simulações
```

## 🔧 Algoritmo Mesh

**Protocolo de roteamento ad-hoc simplificado:**

1. **Route Discovery**: Flooding controlado (RREQ/RREP)
2. **Route Maintenance**: Cache com timeout automático
3. **Data Forwarding**: Multi-hop com TTL control
4. **Loop Prevention**: Sequence numbers + TTL

**Tipos de mensagem:**
- `NEIGHBOR_DISCOVERY/RESPONSE` - Descoberta de vizinhos
- `ROUTE_REQUEST/REPLY` - Descoberta de rotas  
- `MESH_DATA` - Dados com roteamento multi-hop
- `SENSOR_DATA` - Dados diretos (fallback)
- Broadcast periódico a cada 10s ± 3s (randomizado)
- Verificação automática de distância
- Timeout de vizinhos após 30s sem contato
- Logs detalhados de conectividade

### ✅ **Transmissão de Dados**  
# FANET - Flying Ad-hoc Network Simulation

Simulação de rede ad-hoc para drones (UAVs) usando OMNeT++ 6.2.0 e INET 4.5.4.

## 🎯 Visão Geral

Protocolo FANET com **algoritmo de mesh routing** para comunicação multi-hop entre UAVs e Ground Control Station (GCS).

### Características Principais
- **Mesh routing ad-hoc** com descoberta automática de rotas
- **Multi-hop communication** através de UAVs intermediários 
- **Verificação de distância física** para realismo
- **Finalização automática** (evita loops infinitos)
- **Área de simulação**: 2000x2000m

## 🚀 Como Usar

### Compilação e Execução
```bash
# Compilar
make clean && make

# Executar configurações
./run.sh -c Default      # 2 UAVs, 45s (padrão)
./run.sh -c Quick        # 2 UAVs, 30s (teste relay)
./run.sh -c Production   # 5 UAVs, 120s (rede completa)
./run.sh -c FastTest     # 1 UAV, 20s (teste rápido)
```

## 📊 Resultados Típicos

### Configuration Production (5 UAVs)
```
GCS: sent=7 recv=28 data_recv=10 neighbors=3
UAV[0]: sent=4 recv=30 data_sent=2 neighbors=1
UAV[1]: sent=6 recv=33 data_sent=2 neighbors=1  
UAV[2]: sent=6 recv=31 data_sent=2 neighbors=1
UAV[3]: sent=3 recv=28 data_sent=2 neighbors=0
UAV[4]: sent=4 recv=30 data_sent=2 neighbors=1
```

**✅ Sucessos:**
- GCS recebeu 10 pacotes de dados de múltiplos UAVs
- Todos os UAVs conseguiram enviar dados (mesh routing funcionando)
- Rede mesh conectou UAVs distantes através de relays

## ⚙️ Configurações

### Cenários Disponíveis

| Config | UAVs | Duração | Propósito |
|--------|------|---------|-----------|
| `Default` | 2 | 45s | Teste básico |
| `Quick` | 2 | 30s | Teste relay |
| `Production` | 5 | 120s | Rede completa |
| `FastTest` | 1 | 20s | Teste rápido |

### Parâmetros Principais
- **Alcance UAV**: 500m
- **Alcance GCS**: 600m  
- **Velocidade**: 10-15 m/s
- **Área**: 200m-1800m
- **Altitude**: 95-105m

## 📁 Estrutura

```
src/
├── FANETApp.cc/h     # Protocolo FANET com mesh routing
├── Aircraft.cc/h/ned # Modelo de UAV
├── ArbitraryMobility.cc/h/ned # Mobilidade aérea
└── GCS.ned           # Ground Control Station

simulations/
├── omnetpp.ini       # Configurações de simulação
├── FANET.ned         # Topologia da rede
└── results/          # Resultados das simulações
```

## 🔧 Algoritmo Mesh

**Protocolo de roteamento ad-hoc simplificado:**

1. **Route Discovery**: Flooding controlado (RREQ/RREP)
2. **Route Maintenance**: Cache com timeout automático
3. **Data Forwarding**: Multi-hop com TTL control
4. **Loop Prevention**: Sequence numbers + TTL

**Tipos de mensagem:**
- `NEIGHBOR_DISCOVERY/RESPONSE` - Descoberta de vizinhos
- `ROUTE_REQUEST/REPLY` - Descoberta de rotas  
- `MESH_DATA` - Dados com roteamento multi-hop
- `SENSOR_DATA` - Dados diretos (fallback)

## 🛠️ Requisitos

- OMNeT++ 6.2.0+
- INET Framework 4.5.4+
- Sistema Unix/Linux/macOS

---

**Desenvolvido para simulação realística de redes FANET com roteamento mesh automático.**
