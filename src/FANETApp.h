/*
 * FANETApp.h
 * 
 * Aplicação FANET (Flying Ad-hoc Network)
 * Implementa comunicação UDP entre UAVs e Estação de Controle Terrestre (GCS)
 * 
 * Funcionalidades principais:
 * - Descoberta automática de vizinhos via broadcast
 * - Transmissão de dados de sensores dos UAVs para GCS
 * - Relay de dados para UAVs fora do alcance direto da GCS
 * - Monitoramento de conectividade da rede
 */

#ifndef FANETAPP_H_
#define FANETAPP_H_

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include <map>
#include <vector>

using namespace omnetpp;
using namespace inet;

/*
 * Tipos de mensagens suportadas pelo protocolo FANET
 */
enum FANETMessageType {
    NEIGHBOR_DISCOVERY = 1,    // Descoberta de vizinhos (broadcast)
    NEIGHBOR_RESPONSE = 2,     // Resposta à descoberta
    SENSOR_DATA = 3,          // Dados de sensores (UAV -> GCS)
    DATA_RELAY = 4            // Relay de dados (UAV -> UAV -> GCS)
};

/*
 * Estrutura para armazenar informações dos vizinhos na rede FANET
 */
struct NeighborInfo {
    L3Address address;      // Endereço IP do vizinho
    Coord position;         // Posição geográfica
    simtime_t lastSeen;     // Última vez que foi visto
    double distance;        // Distância calculada
    bool isGCS;            // Se é uma Estação de Controle Terrestre
    
    NeighborInfo() : lastSeen(0), distance(0), isGCS(false) {}
    NeighborInfo(L3Address addr, Coord pos, simtime_t time, bool gcs = false) 
        : address(addr), position(pos), lastSeen(time), distance(0), isGCS(gcs) {}
};

/*
 * Aplicação FANET - Flying Ad-hoc Network
 * 
 * Responsabilidades:
 * 1. Descoberta e manutenção de vizinhos
 * 2. Transmissão de dados de sensores (UAV -> GCS)
 * 3. Relay de dados para nós fora de alcance
 * 4. Monitoramento de conectividade
 */
class INET_API FANETApp : public ApplicationBase, public UdpSocket::ICallback {
private:
    // === CONFIGURAÇÃO ===
    int localPort;                    // Porta UDP local
    int destPort;                     // Porta UDP de destino
    simtime_t startTime;              // Tempo de início da aplicação
    double neighborTimeout;           // Timeout para vizinhos (segundos)
    double maxTransmissionRange;      // Alcance máximo de transmissão (metros)
    bool isGCS;                      // Se este nó é uma GCS
    
    // === COMUNICAÇÃO ===
    UdpSocket socket;                // Socket UDP para comunicação
    IMobility *mobility;             // Interface de mobilidade
    
    // === TIMERS ===
    cMessage *neighborDiscoveryTimer;  // Timer para descoberta de vizinhos
    cMessage *dataTransmissionTimer;   // Timer para transmissão de dados
    cMessage *connectivityCheckTimer;  // Timer para verificação de conectividade
    
    // === GERENCIAMENTO DE VIZINHOS ===
    std::map<L3Address, NeighborInfo> neighbors;  // Lista de vizinhos conhecidos
    
    // === ESTATÍSTICAS ===
    simsignal_t packetsReceivedSignal;
    simsignal_t packetsSentSignal;
    simsignal_t dataPacketsReceivedSignal;
    simsignal_t dataPacketsSentSignal;
    simsignal_t neighborsFoundSignal;
    
    int packetsReceived;
    int packetsSent;
    int dataPacketsReceived;
    int dataPacketsSent;

protected:
    // === LIFECYCLE ===
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    
    // === INICIALIZAÇÃO ===
    virtual void configureSocket();
    virtual void scheduleTimers();
    
    // === PROCESSAMENTO DE MENSAGENS ===
    virtual void processNeighborDiscovery(cPacket *packet, L3Address senderAddr);
    virtual void processNeighborResponse(cPacket *packet, L3Address senderAddr);
    virtual void processSensorData(cPacket *packet, L3Address senderAddr);
    virtual void processDataRelay(cPacket *packet, L3Address senderAddr);
    
    // === FUNCIONALIDADES CORE FANET ===
    virtual void sendNeighborDiscovery();
    virtual void sendSensorData();
    virtual void sendDataRelay(cPacket *originalPacket, L3Address nextHop);
    virtual void checkConnectivity();
    
    // === UTILITÁRIOS ===
    virtual L3Address findGCSAddress();
    virtual L3Address findBestRelayToGCS();
    virtual double calculateDistance(const Coord& pos1, const Coord& pos2);
    virtual void updateNeighborInfo(L3Address address, Coord position, bool isGCS = false);
    virtual void cleanExpiredNeighbors();
    virtual cPacket* createFANETMessage(FANETMessageType type, const char* data = nullptr);
    
    // === LIFECYCLE INET 4.5.4 ===
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_APPLICATION_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == INITSTAGE_APPLICATION_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == INITSTAGE_APPLICATION_LAYER; }
    
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
    
    // === CALLBACKS UDP ===
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

public:
    FANETApp() {}
    virtual ~FANETApp();
};

#endif /* FANETAPP_H_ */
