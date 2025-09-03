/*
 * FANETApp.h - VERSÃO CORRIGIDA
 * 
 * Header file otimizado para protocolo FANET
 * Principais melhorias:
 * 1. Constantes bem definidas
 * 2. Estruturas de dados simplificadas  
 * 3. Gerenciamento de timers com limites
 * 4. Interface limpa e bem documentada
 */

#ifndef FANETAPP_H_
#define FANETAPP_H_

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include <map>
#include <string>

using namespace omnetpp;
using namespace inet;

// =============================================================================
// ENUMS E ESTRUTURAS
// =============================================================================

// Tipos de mensagens do protocolo FANET
enum FANETMessageType {
    NEIGHBOR_DISCOVERY = 1,
    NEIGHBOR_RESPONSE = 2,
    SENSOR_DATA = 3,
    DATA_RELAY = 4,
    ROUTE_REQUEST = 5,      // Novo: Requisição de rota (RREQ)
    ROUTE_REPLY = 6,        // Novo: Resposta de rota (RREP)
    MESH_DATA = 7           // Novo: Dados com roteamento mesh
};

// Informações sobre vizinhos
struct NeighborInfo {
    L3Address address;
    Coord position;
    simtime_t lastSeen;
    bool isGCS;
    double distance;
    
    NeighborInfo() : lastSeen(0), isGCS(false), distance(0) {}
};

// Entrada da tabela de roteamento mesh
struct MeshRouteEntry {
    L3Address destination;     // Destino final
    L3Address nextHop;         // Próximo salto
    int hopCount;             // Número de saltos
    simtime_t timestamp;      // Quando a rota foi descoberta
    bool isValid;             // Se a rota ainda é válida
    
    MeshRouteEntry() : hopCount(0), timestamp(0), isValid(false) {}
};

// Cache de requisições de rota para evitar flooding excessivo
struct RouteRequestEntry {
    L3Address originator;     // Quem originou a requisição
    int sequenceNumber;       // Número de sequência da requisição
    simtime_t timestamp;      // Quando foi vista
    
    RouteRequestEntry() : sequenceNumber(0), timestamp(0) {}
};

// =============================================================================
// CLASSE PRINCIPAL
// =============================================================================

class INET_API FANETApp : public ApplicationBase, public UdpSocket::ICallback {
public:
    FANETApp();
    virtual ~FANETApp();

protected:
    // =============================================================================
    // CONSTANTES DO PROTOCOLO
    // =============================================================================
    static const int MAX_DISCOVERY_ROUNDS;
    static const int MAX_DATA_TRANSMISSIONS;
    static const int MAX_CONNECTIVITY_CHECKS;
    static const int MAX_HOP_COUNT;
    static const double DISCOVERY_INTERVAL;
    static const double DATA_INTERVAL;
    static const double CONNECTIVITY_INTERVAL;

    // =============================================================================
    // PARÂMETROS DE CONFIGURAÇÃO
    // =============================================================================
    int localPort, destPort;
    simtime_t startTime;
    simtime_t neighborTimeout;
    double maxTransmissionRange;
    bool isGCS;

    // =============================================================================
    // CONTADORES E ESTATÍSTICAS
    // =============================================================================
    int discoveryCount;
    int dataTransmissionCount;
    int connectivityCheckCount;
    
    int packetsReceived;
    int packetsSent;
    int dataPacketsReceived;
    int dataPacketsSent;
    int relayAttempts;
    int successfulRelays;
    int droppedPackets;

    // =============================================================================
    // MESH ROUTING VARIABLES
    // =============================================================================
    std::map<L3Address, MeshRouteEntry> routingTable;     // Tabela de roteamento
    std::map<std::pair<L3Address, int>, RouteRequestEntry> rreqCache;  // Cache de RREQ
    int sequenceNumber;                                   // Número de sequência local
    static const int MAX_TTL = 5;                        // TTL máximo para evitar loops
    static constexpr double ROUTE_TIMEOUT = 60.0;       // Timeout das rotas em segundos

    // =============================================================================
    // COMPONENTES DO SISTEMA
    // =============================================================================
    UdpSocket socket;
    IMobility *mobility;
    L3Address myAddr;                                     // Endereço IP deste nó
    std::map<L3Address, NeighborInfo> neighbors;

    // Timers
    cMessage *neighborDiscoveryTimer;
    cMessage *dataTransmissionTimer;
    cMessage *connectivityCheckTimer;
    cMessage *finalizationTimer;

    // Sinais para estatísticas
    simsignal_t packetsReceivedSignal;
    simsignal_t packetsSentSignal;
    simsignal_t dataPacketsReceivedSignal;
    simsignal_t dataPacketsSentSignal;
    simsignal_t neighborsFoundSignal;

    // =============================================================================
    // MÉTODOS LIFECYCLE
    // =============================================================================
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    // Configuração inicial
    virtual void configureSocket();
    virtual void scheduleInitialTimers();
    virtual void forceFinalization();

    // =============================================================================
    // PROTOCOLO FANET CORE
    // =============================================================================
    virtual void sendNeighborDiscovery();
    virtual void sendSensorData();
    virtual void sendDirectToGCS(const std::string& data, const L3Address& gcsAddr);
    virtual void sendViaRelay(const std::string& data, const L3Address& relayAddr);
    virtual void sendDataBroadcast(const std::string& data);
    virtual void checkConnectivity();

    // =============================================================================
    // MESH ROUTING FUNCTIONS
    // =============================================================================
    virtual void sendRouteRequest(const L3Address& destination);
    virtual void sendRouteReply(const L3Address& destination, const L3Address& originator, int hopCount);
    virtual void sendMeshData(const std::string& data, const L3Address& destination);
    virtual bool forwardMeshData(cPacket* packet, const L3Address& destination, int ttl);
    virtual void processRouteRequest(cPacket *packet, L3Address senderAddr);
    virtual void processRouteReply(cPacket *packet, L3Address senderAddr);
    virtual void processMeshData(cPacket *packet, L3Address senderAddr);
    virtual void updateRoutingTable(const L3Address& destination, const L3Address& nextHop, int hopCount);
    virtual L3Address findRoute(const L3Address& destination);
    virtual void cleanExpiredRoutes();
    virtual bool isRouteRequestSeen(const L3Address& originator, int seqNum);

    // =============================================================================
    // PROCESSAMENTO DE MENSAGENS
    // =============================================================================
    virtual void processNeighborDiscovery(cPacket *packet, L3Address senderAddr);
    virtual void processNeighborResponse(cPacket *packet, L3Address senderAddr);
    virtual void processSensorData(cPacket *packet, L3Address senderAddr);
    virtual void processDataRelay(cPacket *packet, L3Address senderAddr);

    // =============================================================================
    // UTILITÁRIOS E HELPERS
    // =============================================================================
    virtual L3Address findDirectGCSConnection();
    virtual L3Address findBestRelay();
    virtual L3Address findGCSInNetwork();  // Novo: busca GCS na rede mesh
    virtual std::string generateSensorData(const Coord& pos);
    virtual void addPositionInfo(cPacket* packet, const Coord& pos);
    virtual cPacket* createFANETMessage(FANETMessageType type);
    virtual void cleanExpiredNeighbors();
    virtual double calculateDistance(const Coord& pos1, const Coord& pos2);
    virtual bool parsePositionInfo(const std::string& posStr, Coord& pos, bool& isGCSNode);
    virtual void updateNeighborInfo(L3Address address, Coord position, bool neighborIsGCS);

    // =============================================================================
    // LIFECYCLE OPERATIONS (REQUIRED BY INET)
    // =============================================================================
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // =============================================================================
    // CALLBACKS UDP
    // =============================================================================
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;
};

#endif /* FANETAPP_H_ */