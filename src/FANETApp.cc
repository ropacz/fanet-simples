/*
 * FANETApp.cc - VERSÃO CORRIGIDA
 * 
 * Implementação otimizada de protocolo FANET para drones
 * Correções aplicadas:
 * 1. Timers com limite de execuções (evita loops infinitos)
 * 2. Gerenciamento de memória melhorado
 * 3. Lógica de relay multi-hop simplificada
 * 4. Protocolo de descoberta de vizinhos mais eficiente
 * 5. Remoção de código duplicado
 */

#include "FANETApp.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/cPacketChunk.h"
#include <sstream>

using namespace omnetpp;
using namespace inet;

Define_Module(FANETApp);

// =============================================================================
// CONSTANTES DO PROTOCOLO
// =============================================================================
const int FANETApp::MAX_DISCOVERY_ROUNDS = 3;
const int FANETApp::MAX_DATA_TRANSMISSIONS = 4; 
const int FANETApp::MAX_CONNECTIVITY_CHECKS = 2;
const int FANETApp::MAX_HOP_COUNT = 3;
const double FANETApp::DISCOVERY_INTERVAL = 15.0;
const double FANETApp::DATA_INTERVAL = 20.0;
const double FANETApp::CONNECTIVITY_INTERVAL = 40.0;

// =============================================================================
// MESH ROUTING IMPLEMENTATION
// =============================================================================
void FANETApp::sendRouteRequest(const L3Address& destination) {
    sequenceNumber++;
    
    cPacket *packet = createFANETMessage(ROUTE_REQUEST);
    packet->addPar("destination") = destination.str().c_str();
    packet->addPar("originator") = getModuleByPath("^.ipv4.ip")->par("address").stringValue();
    packet->addPar("sequenceNumber") = sequenceNumber;
    packet->addPar("hopCount") = 0;
    packet->addPar("ttl") = MAX_TTL;
    
    Packet *rreqPacket = new Packet("FANETRouteRequest");
    rreqPacket->insertAtBack(makeShared<cPacketChunk>(packet));
    socket.sendTo(rreqPacket, Ipv4Address::ALLONES_ADDRESS, destPort);
    
    packetsSent++;
    emit(packetsSentSignal, packetsSent);
    
    EV << "Route request sent for destination " << destination << " (seq: " << sequenceNumber << ")" << endl;
}

void FANETApp::sendMeshData(const std::string& data, const L3Address& destination) {
    L3Address nextHop = findRoute(destination);
    
    if (!nextHop.isUnspecified()) {
        // Temos uma rota conhecida
        cPacket *packet = createFANETMessage(MESH_DATA);
        packet->addPar("data") = data.c_str();
        packet->addPar("sourceUAV") = getIndex();
        packet->addPar("destination") = destination.str().c_str();
        packet->addPar("ttl") = MAX_TTL;
        
        Packet *meshPacket = new Packet("FANETMeshData");
        meshPacket->insertAtBack(makeShared<cPacketChunk>(packet));
        socket.sendTo(meshPacket, nextHop, destPort);
        
        dataPacketsSent++;
        emit(dataPacketsSentSignal, dataPacketsSent);
        
        EV << "Mesh data sent: UAV" << getIndex() << " → " << nextHop << " → " << destination << endl;
    } else {
        // Não temos rota, fazer route discovery
        EV << "No route to " << destination << ", initiating route discovery" << endl;
        sendRouteRequest(destination);
        
        // Guardar dados para reenvio após descoberta (simplificado: enviar broadcast)
        sendDataBroadcast(data);
    }
}

L3Address FANETApp::findGCSInNetwork() {
    // Primeiro verificar vizinhos diretos
    for (const auto& neighbor : neighbors) {
        if (neighbor.second.isGCS) {
            return neighbor.first;
        }
    }
    
    // Verificar tabela de roteamento
    for (const auto& route : routingTable) {
        if (route.second.isValid) {
            // Assumir que qualquer rota válida pode levar ao GCS
            // (simplificação para o algoritmo básico)
            return route.first;
        }
    }
    
    return L3Address(); // Não encontrado
}

void FANETApp::updateRoutingTable(const L3Address& destination, const L3Address& nextHop, int hopCount) {
    auto& entry = routingTable[destination];
    
    // Atualizar apenas se for uma rota melhor ou nova
    if (!entry.isValid || hopCount < entry.hopCount) {
        entry.destination = destination;
        entry.nextHop = nextHop;
        entry.hopCount = hopCount;
        entry.timestamp = simTime();
        entry.isValid = true;
        
        EV << "Route updated: " << destination << " via " << nextHop << " (" << hopCount << " hops)" << endl;
    }
}

L3Address FANETApp::findRoute(const L3Address& destination) {
    cleanExpiredRoutes();
    
    auto it = routingTable.find(destination);
    if (it != routingTable.end() && it->second.isValid) {
        return it->second.nextHop;
    }
    
    return L3Address(); // Não encontrada
}

void FANETApp::cleanExpiredRoutes() {
    simtime_t now = simTime();
    auto it = routingTable.begin();
    
    while (it != routingTable.end()) {
        if (now - it->second.timestamp > ROUTE_TIMEOUT) {
            it->second.isValid = false;
            EV << "Route to " << it->first << " expired" << endl;
        }
        ++it;
    }
}

bool FANETApp::isRouteRequestSeen(const L3Address& originator, int seqNum) {
    auto key = std::make_pair(originator, seqNum);
    auto it = rreqCache.find(key);
    
    if (it != rreqCache.end()) {
        return true; // Já vimos esta requisição
    }
    
    // Adicionar ao cache
    rreqCache[key] = RouteRequestEntry();
    rreqCache[key].originator = originator;
    rreqCache[key].sequenceNumber = seqNum;
    rreqCache[key].timestamp = simTime();
    
    return false; // Primeira vez vendo esta requisição
}

void FANETApp::processRouteRequest(cPacket *packet, L3Address senderAddr) {
    if (!packet->hasPar("destination") || !packet->hasPar("originator") || 
        !packet->hasPar("sequenceNumber") || !packet->hasPar("hopCount") || !packet->hasPar("ttl")) {
        return;
    }
    
    L3Address destination = L3Address(packet->par("destination").stringValue());
    L3Address originator = L3Address(packet->par("originator").stringValue());
    int seqNum = packet->par("sequenceNumber");
    int hopCount = packet->par("hopCount");
    int ttl = packet->par("ttl");
    
    // Verificar se já processamos esta requisição
    if (isRouteRequestSeen(originator, seqNum) || ttl <= 0) {
        return;
    }
    
    // Atualizar tabela de roteamento para o originador
    updateRoutingTable(originator, senderAddr, hopCount + 1);
    
    // Se somos o destino, enviar RREP
    L3Address myAddr = L3Address(getModuleByPath("^.ipv4.ip")->par("address").stringValue());
    if (destination == myAddr) {
        sendRouteReply(originator, myAddr, 0);
        return;
    }
    
    // Verificar se temos rota para o destino
    L3Address routeToDestination = findRoute(destination);
    if (!routeToDestination.isUnspecified()) {
        // Temos rota, enviar RREP
        auto routeEntry = routingTable.find(destination);
        if (routeEntry != routingTable.end()) {
            sendRouteReply(originator, myAddr, routeEntry->second.hopCount);
        }
        return;
    }
    
    // Propagar RREQ se TTL > 0
    if (ttl > 1) {
        cPacket *newPacket = createFANETMessage(ROUTE_REQUEST);
        newPacket->addPar("destination") = destination.str().c_str();
        newPacket->addPar("originator") = originator.str().c_str();
        newPacket->addPar("sequenceNumber") = seqNum;
        newPacket->addPar("hopCount") = hopCount + 1;
        newPacket->addPar("ttl") = ttl - 1;
        
        Packet *rreqPacket = new Packet("FANETRouteRequest");
        rreqPacket->insertAtBack(makeShared<cPacketChunk>(newPacket));
        socket.sendTo(rreqPacket, Ipv4Address::ALLONES_ADDRESS, destPort);
        
        packetsSent++;
        emit(packetsSentSignal, packetsSent);
        
        EV << "Route request forwarded for " << destination << " (ttl: " << (ttl-1) << ")" << endl;
    }
}

void FANETApp::processRouteReply(cPacket *packet, L3Address senderAddr) {
    if (!packet->hasPar("destination") || !packet->hasPar("originator") || !packet->hasPar("hopCount")) {
        return;
    }
    
    L3Address destination = L3Address(packet->par("destination").stringValue());
    L3Address originator = L3Address(packet->par("originator").stringValue());
    int hopCount = packet->par("hopCount");
    
    // Atualizar tabela de roteamento
    updateRoutingTable(destination, senderAddr, hopCount + 1);
    
    EV << "Route reply processed: route to " << destination << " via " << senderAddr << endl;
}

void FANETApp::processMeshData(cPacket *packet, L3Address senderAddr) {
    if (!packet->hasPar("destination") || !packet->hasPar("ttl")) {
        return;
    }
    
    L3Address destination = L3Address(packet->par("destination").stringValue());
    int ttl = packet->par("ttl");
    L3Address myAddr = L3Address(getModuleByPath("^.ipv4.ip")->par("address").stringValue());
    
    // Se somos o destino
    if (destination == myAddr) {
        if (isGCS && packet->hasPar("data")) {
            dataPacketsReceived++;
            emit(dataPacketsReceivedSignal, dataPacketsReceived);
            EV << "Mesh data received at GCS from UAV" << packet->par("sourceUAV").longValue() << endl;
        }
        return;
    }
    
    // Propagar dados se TTL > 0
    if (ttl > 1 && forwardMeshData(packet, destination, ttl - 1)) {
        EV << "Mesh data forwarded towards " << destination << endl;
    } else {
        droppedPackets++;
        EV << "Mesh data dropped (no route or TTL=0)" << endl;
    }
}

bool FANETApp::forwardMeshData(cPacket* packet, const L3Address& destination, int ttl) {
    L3Address nextHop = findRoute(destination);
    
    if (!nextHop.isUnspecified()) {
        // Criar nova mensagem com TTL decrementado
        cPacket *newPacket = createFANETMessage(MESH_DATA);
        newPacket->addPar("data") = packet->par("data").stringValue();
        newPacket->addPar("sourceUAV") = packet->par("sourceUAV").longValue();
        newPacket->addPar("destination") = destination.str().c_str();
        newPacket->addPar("ttl") = ttl;
        
        Packet *meshPacket = new Packet("FANETMeshData");
        meshPacket->insertAtBack(makeShared<cPacketChunk>(newPacket));
        socket.sendTo(meshPacket, nextHop, destPort);
        
        relayAttempts++;
        successfulRelays++;
        packetsSent++;
        emit(packetsSentSignal, packetsSent);
        
        return true;
    }
    
    return false;
}

void FANETApp::sendRouteReply(const L3Address& destination, const L3Address& originator, int hopCount) {
    cPacket *packet = createFANETMessage(ROUTE_REPLY);
    packet->addPar("destination") = destination.str().c_str();
    packet->addPar("originator") = originator.str().c_str();
    packet->addPar("hopCount") = hopCount;
    
    // Enviar RREP de volta ao originador
    L3Address nextHopToOriginator = findRoute(originator);
    if (!nextHopToOriginator.isUnspecified()) {
        Packet *rrepPacket = new Packet("FANETRouteReply");
        rrepPacket->insertAtBack(makeShared<cPacketChunk>(packet));
        socket.sendTo(rrepPacket, nextHopToOriginator, destPort);
        
        packetsSent++;
        emit(packetsSentSignal, packetsSent);
        
        EV << "Route reply sent to " << originator << " via " << nextHopToOriginator << endl;
    }
}

// =============================================================================
// LIFECYCLE OPERATIONS (REQUIRED BY INET)
FANETApp::FANETApp() : 
    discoveryCount(0), dataTransmissionCount(0), connectivityCheckCount(0),
    packetsReceived(0), packetsSent(0), dataPacketsReceived(0), dataPacketsSent(0),
    relayAttempts(0), successfulRelays(0), droppedPackets(0), sequenceNumber(0),
    neighborDiscoveryTimer(nullptr), dataTransmissionTimer(nullptr), 
    connectivityCheckTimer(nullptr), finalizationTimer(nullptr) {
}

FANETApp::~FANETApp() {
    cancelAndDelete(neighborDiscoveryTimer);
    cancelAndDelete(dataTransmissionTimer);
    cancelAndDelete(connectivityCheckTimer);
    cancelAndDelete(finalizationTimer);
}

void FANETApp::initialize(int stage) {
    ApplicationBase::initialize(stage);
    
    if (stage == INITSTAGE_LOCAL) {
        // Parâmetros de configuração
        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        neighborTimeout = par("neighborTimeout");
        maxTransmissionRange = par("maxTransmissionRange");
        isGCS = par("isGCS");
        
        // Registro de sinais para estatísticas
        packetsReceivedSignal = registerSignal("packetsReceived");
        packetsSentSignal = registerSignal("packetsSent");
        dataPacketsReceivedSignal = registerSignal("dataPacketsReceived");
        dataPacketsSentSignal = registerSignal("dataPacketsSent");
        neighborsFoundSignal = registerSignal("neighborsFound");
        
        // Criação de timers
        neighborDiscoveryTimer = new cMessage("neighborDiscoveryTimer");
        dataTransmissionTimer = new cMessage("dataTransmissionTimer");
        connectivityCheckTimer = new cMessage("connectivityCheckTimer");
        finalizationTimer = new cMessage("finalizationTimer");
        
        // Módulo de mobilidade
        mobility = check_and_cast<IMobility *>(getModuleByPath("^.mobility"));
        
        EV << "FANET " << (isGCS ? "GCS" : "UAV") << " " << getIndex() 
           << " initialized with maxRange=" << maxTransmissionRange << "m" << endl;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        configureSocket();
        scheduleInitialTimers();
    }
}

void FANETApp::configureSocket() {
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    socket.setBroadcast(true);
    socket.setCallback(this);
}

void FANETApp::scheduleInitialTimers() {
    double startDelay = uniform(1.0, 3.0);
    
    // Timer de descoberta inicial
    scheduleAt(startTime + startDelay, neighborDiscoveryTimer);
    
    // Timer de dados (apenas para UAVs)
    if (!isGCS) {
        scheduleAt(startTime + startDelay + 5.0, dataTransmissionTimer);
    }
    
    // Timer de conectividade
    scheduleAt(startTime + startDelay + 8.0, connectivityCheckTimer);
    
    // CRÍTICO: Timer de finalização forçada para evitar loops infinitos
    scheduleAt(startTime + 45.0, finalizationTimer);
    
    EV << "Timers scheduled with finalization at t=" << (startTime + 45.0) << endl;
}

// =============================================================================
// GERENCIAMENTO DE MENSAGENS (CORRIGIDO)
// =============================================================================
void FANETApp::handleMessageWhenUp(cMessage *msg) {
    // Timer de finalização forçada - PRIORIDADE MÁXIMA
    if (msg == finalizationTimer) {
        EV << "Forced finalization at t=" << simTime() << endl;
        forceFinalization();
        return;
    }
    
    // Discovery com limite de execuções
    if (msg == neighborDiscoveryTimer) {
        if (discoveryCount < MAX_DISCOVERY_ROUNDS) {
            sendNeighborDiscovery();
            discoveryCount++;
            scheduleAt(simTime() + DISCOVERY_INTERVAL, neighborDiscoveryTimer);
            EV << "Discovery " << discoveryCount << "/" << MAX_DISCOVERY_ROUNDS << " completed" << endl;
        } else {
            EV << "Discovery rounds completed, stopping timer" << endl;
        }
        return;
    }
    
    // Transmissão de dados com limite
    if (msg == dataTransmissionTimer) {
        if (dataTransmissionCount < MAX_DATA_TRANSMISSIONS && !isGCS) {
            sendSensorData();
            dataTransmissionCount++;
            scheduleAt(simTime() + DATA_INTERVAL, dataTransmissionTimer);
            EV << "Data transmission " << dataTransmissionCount << "/" << MAX_DATA_TRANSMISSIONS << " completed" << endl;
        } else {
            EV << "Data transmissions completed, stopping timer" << endl;
        }
        return;
    }
    
    // Verificação de conectividade com limite
    if (msg == connectivityCheckTimer) {
        if (connectivityCheckCount < MAX_CONNECTIVITY_CHECKS) {
            checkConnectivity();
            connectivityCheckCount++;
            scheduleAt(simTime() + CONNECTIVITY_INTERVAL, connectivityCheckTimer);
            EV << "Connectivity check " << connectivityCheckCount << "/" << MAX_CONNECTIVITY_CHECKS << " completed" << endl;
        } else {
            EV << "Connectivity checks completed, stopping timer" << endl;
        }
        return;
    }
    
    // Mensagens desconhecidas
    if (msg->isSelfMessage()) {
        EV << "WARNING: Unknown self-message: " << msg->getName() << endl;
        delete msg;
        return;
    }
    
    // Processamento UDP
    socket.processMessage(msg);
}

void FANETApp::forceFinalization() {
    // Cancelar todos os timers pendentes
    cancelAndDelete(neighborDiscoveryTimer);
    cancelAndDelete(dataTransmissionTimer);
    cancelAndDelete(connectivityCheckTimer);
    
    neighborDiscoveryTimer = nullptr;
    dataTransmissionTimer = nullptr;
    connectivityCheckTimer = nullptr;
    
    EV << "All timers cancelled, simulation will end naturally" << endl;
    delete finalizationTimer;
    finalizationTimer = nullptr;
}

// =============================================================================
// PROTOCOLO FANET CORE (OTIMIZADO)
// =============================================================================
void FANETApp::sendNeighborDiscovery() {
    Coord myPos = mobility->getCurrentPosition();
    
    // Criar mensagem de descoberta
    cPacket *packet = createFANETMessage(NEIGHBOR_DISCOVERY);
    addPositionInfo(packet, myPos);
    
    // Broadcast
    Packet *discoveryPacket = new Packet("FANETDiscovery");
    discoveryPacket->insertAtBack(makeShared<cPacketChunk>(packet));
    
    socket.sendTo(discoveryPacket, Ipv4Address::ALLONES_ADDRESS, destPort);
    
    packetsSent++;
    emit(packetsSentSignal, packetsSent);
    
    EV << "Discovery broadcast sent from " << (isGCS ? "GCS" : "UAV") << " " << getIndex() << endl;
}

void FANETApp::sendSensorData() {
    if (isGCS) return; // Apenas UAVs enviam dados de sensores
    
    Coord myPos = mobility->getCurrentPosition();
    
    // Dados de sensores simulados
    std::string sensorData = generateSensorData(myPos);
    
    // NOVO: Usar mesh routing para encontrar GCS
    L3Address gcsAddr = findGCSInNetwork();
    
    if (!gcsAddr.isUnspecified()) {
        // Verificar se temos rota direta
        L3Address directGCS = findDirectGCSConnection();
        if (!directGCS.isUnspecified()) {
            sendDirectToGCS(sensorData, directGCS);
            EV << "UAV" << getIndex() << " sending data DIRECT to GCS" << endl;
        } else {
            // Usar mesh routing
            sendMeshData(sensorData, gcsAddr);
            EV << "UAV" << getIndex() << " sending data via MESH to GCS" << endl;
        }
    } else {
        // Fallback: broadcast se não encontrar GCS
        EV << "UAV" << getIndex() << " no GCS found, trying BROADCAST" << endl;
        sendDataBroadcast(sensorData);
    }
}

void FANETApp::sendDirectToGCS(const std::string& data, const L3Address& gcsAddr) {
    cPacket *packet = createFANETMessage(SENSOR_DATA);
    packet->addPar("data") = data.c_str();
    packet->addPar("sourceUAV") = getIndex();
    
    Packet *sensorPacket = new Packet("FANETSensorData");
    sensorPacket->insertAtBack(makeShared<cPacketChunk>(packet));
    socket.sendTo(sensorPacket, gcsAddr, destPort);
    
    dataPacketsSent++;
    emit(dataPacketsSentSignal, dataPacketsSent);
    
    EV << "Sensor data sent: UAV" << getIndex() << " → GCS (direct)" << endl;
}

void FANETApp::sendViaRelay(const std::string& data, const L3Address& relayAddr) {
    cPacket *packet = createFANETMessage(DATA_RELAY);
    packet->addPar("data") = data.c_str();
    packet->addPar("sourceUAV") = getIndex();
    packet->addPar("hopCount") = 1;
    
    Packet *relayPacket = new Packet("FANETDataRelay");
    relayPacket->insertAtBack(makeShared<cPacketChunk>(packet));
    socket.sendTo(relayPacket, relayAddr, destPort);
    
    relayAttempts++;
    successfulRelays++;
    packetsSent++;
    emit(packetsSentSignal, packetsSent);
    
    EV << "Sensor data sent: UAV" << getIndex() << " → " << relayAddr << " → GCS (relay)" << endl;
}

void FANETApp::sendDataBroadcast(const std::string& data) {
    cPacket *packet = createFANETMessage(SENSOR_DATA);
    packet->addPar("data") = data.c_str();
    packet->addPar("sourceUAV") = getIndex();
    packet->addPar("hopCount") = 1;
    
    Packet *broadcastPacket = new Packet("FANETDataBroadcast");
    broadcastPacket->insertAtBack(makeShared<cPacketChunk>(packet));
    socket.sendTo(broadcastPacket, Ipv4Address::ALLONES_ADDRESS, destPort);
    
    dataPacketsSent++;
    emit(dataPacketsSentSignal, dataPacketsSent);
    
    EV << "Sensor data sent: UAV" << getIndex() << " → BROADCAST (no neighbors)" << endl;
}

void FANETApp::checkConnectivity() {
    cleanExpiredNeighbors();
    
    int totalNeighbors = neighbors.size();
    int gcsConnected = 0;
    int uavNeighbors = 0;
    
    for (const auto& neighbor : neighbors) {
        if (neighbor.second.isGCS) {
            gcsConnected = 1;
        } else {
            uavNeighbors++;
        }
    }
    
    emit(neighborsFoundSignal, totalNeighbors);
    
    EV << "Connectivity: " << (isGCS ? "GCS" : "UAV") << " " << getIndex() 
       << " has " << totalNeighbors << " neighbors (" << uavNeighbors << " UAVs, " 
       << (gcsConnected ? "GCS" : "no GCS") << ")" << endl;
}

// =============================================================================
// UTILITÁRIOS (SIMPLIFICADOS)
// =============================================================================
L3Address FANETApp::findDirectGCSConnection() {
    EV << "UAV" << getIndex() << " looking for GCS among " << neighbors.size() << " neighbors" << endl;
    for (const auto& neighbor : neighbors) {
        EV << "  Neighbor: " << neighbor.first << " isGCS=" << neighbor.second.isGCS 
           << " dist=" << (int)neighbor.second.distance << "m" << endl;
        if (neighbor.second.isGCS && neighbor.second.distance <= maxTransmissionRange) {
            EV << "  -> GCS found at distance " << (int)neighbor.second.distance << "m" << endl;
            return neighbor.first;
        }
    }
    EV << "  -> No direct GCS connection found" << endl;
    return L3Address(); // Não encontrada
}

L3Address FANETApp::findBestRelay() {
    EV << "UAV" << getIndex() << " looking for relay among " << neighbors.size() << " neighbors" << endl;
    L3Address bestRelay;
    double bestScore = -1;
    
    for (const auto& neighbor : neighbors) {
        if (!neighbor.second.isGCS && neighbor.second.distance <= maxTransmissionRange) {
            // Score simples baseado em proximidade e atualidade
            double score = 1.0 / (1.0 + neighbor.second.distance / 100.0);
            score *= 1.0 / (1.0 + (simTime() - neighbor.second.lastSeen).dbl());
            
            EV << "  Potential relay: " << neighbor.first << " score=" << score << endl;
            
            if (score > bestScore) {
                bestScore = score;
                bestRelay = neighbor.first;
            }
        }
    }
    
    if (!bestRelay.isUnspecified()) {
        EV << "  -> Best relay found: " << bestRelay << endl;
    } else {
        EV << "  -> No relay found" << endl;
    }
    
    return bestRelay;
}

std::string FANETApp::generateSensorData(const Coord& pos) {
    char buffer[200];
    sprintf(buffer, "UAV_%d_SENSOR: Pos(%.1f,%.1f,%.1f) Temp:%.1f Bat:%.0f%% T:%.2f", 
            getIndex(), pos.x, pos.y, pos.z, 
            uniform(20, 35), uniform(60, 100), simTime().dbl());
    return std::string(buffer);
}

void FANETApp::addPositionInfo(cPacket* packet, const Coord& pos) {
    char posInfo[100];
    sprintf(posInfo, "%.2f,%.2f,%.2f,%s", 
            pos.x, pos.y, pos.z, isGCS ? "GCS" : "UAV");
    packet->addPar("positionInfo") = posInfo;
}

cPacket* FANETApp::createFANETMessage(FANETMessageType type) {
    char msgName[50];
    sprintf(msgName, "FANET_%d_%d", type, getIndex());
    
    cPacket *packet = new cPacket(msgName);
    packet->addPar("messageType") = type;
    packet->addPar("nodeIndex") = getIndex();
    packet->addPar("timestamp") = simTime().dbl();
    packet->setByteLength(64);
    
    return packet;
}

void FANETApp::cleanExpiredNeighbors() {
    simtime_t now = simTime();
    auto it = neighbors.begin();
    
    while (it != neighbors.end()) {
        if (now - it->second.lastSeen > neighborTimeout) {
            it = neighbors.erase(it);
        } else {
            ++it;
        }
    }
}

double FANETApp::calculateDistance(const Coord& pos1, const Coord& pos2) {
    double dx = pos1.x - pos2.x;
    double dy = pos1.y - pos2.y;
    double dz = pos1.z - pos2.z;
    return sqrt(dx*dx + dy*dy + dz*dz);
}

// =============================================================================
// PROCESSAMENTO DE MENSAGENS (SIMPLIFICADO)  
// =============================================================================
void FANETApp::socketDataArrived(UdpSocket *socket, Packet *packet) {
    packetsReceived++;
    emit(packetsReceivedSignal, packetsReceived);
    
    // Obter endereço do remetente através das tags do pacote
    L3Address senderAddress;
    auto addrInd = packet->findTag<L3AddressInd>();
    if (addrInd != nullptr) {
        senderAddress = addrInd->getSrcAddress();
    } else {
        EV << "Warning: No address indication found in packet" << endl;
        delete packet;
        return;
    }
    
    // Filtrar loopback e próprias mensagens
    if (senderAddress.str() == "127.0.0.1") {
        delete packet;
        return;
    }
    
    // Processar mensagem FANET
    auto chunk = packet->peekAtBack<cPacketChunk>(packet->getDataLength());
    if (!chunk) {
        delete packet;
        return;
    }
    
    cPacket *fanetPacket = chunk->getPacket()->dup();
    int messageType = fanetPacket->par("messageType");
    
    switch (messageType) {
        case NEIGHBOR_DISCOVERY:
            processNeighborDiscovery(fanetPacket, senderAddress);
            break;
        case NEIGHBOR_RESPONSE:
            processNeighborResponse(fanetPacket, senderAddress);
            break;
        case SENSOR_DATA:
            if (isGCS) processSensorData(fanetPacket, senderAddress);
            break;
        case DATA_RELAY:
            processDataRelay(fanetPacket, senderAddress);
            break;
        case ROUTE_REQUEST:
            processRouteRequest(fanetPacket, senderAddress);
            break;
        case ROUTE_REPLY:
            processRouteReply(fanetPacket, senderAddress);
            break;
        case MESH_DATA:
            processMeshData(fanetPacket, senderAddress);
            break;
        default:
            EV << "Unknown FANET message type: " << messageType << endl;
            break;
    }
    
    delete fanetPacket;
    delete packet;
}

void FANETApp::processNeighborDiscovery(cPacket *packet, L3Address senderAddr) {
    if (!packet->hasPar("positionInfo")) return;
    
    Coord senderPos, myPos;
    bool senderIsGCS;
    
    if (!parsePositionInfo(packet->par("positionInfo").stringValue(), senderPos, senderIsGCS)) {
        return;
    }
    
    myPos = mobility->getCurrentPosition();
    double distance = calculateDistance(myPos, senderPos);
    
    if (distance <= maxTransmissionRange) {
        updateNeighborInfo(senderAddr, senderPos, senderIsGCS);
        
        // Enviar resposta
        cPacket *response = createFANETMessage(NEIGHBOR_RESPONSE);
        addPositionInfo(response, myPos);
        
        Packet *responsePacket = new Packet("FANETResponse");
        responsePacket->insertAtBack(makeShared<cPacketChunk>(response));
        socket.sendTo(responsePacket, senderAddr, destPort);
        
        packetsSent++;
        emit(packetsSentSignal, packetsSent);
        
        EV << "Discovery response sent to " << senderAddr << " (dist: " << (int)distance << "m)" << endl;
    }
}

void FANETApp::processNeighborResponse(cPacket *packet, L3Address senderAddr) {
    if (!packet->hasPar("positionInfo")) return;
    
    Coord senderPos;
    bool senderIsGCS;
    
    if (parsePositionInfo(packet->par("positionInfo").stringValue(), senderPos, senderIsGCS)) {
        Coord myPos = mobility->getCurrentPosition();
        double distance = calculateDistance(myPos, senderPos);
        
        if (distance <= maxTransmissionRange) {
            updateNeighborInfo(senderAddr, senderPos, senderIsGCS);
            EV << "Neighbor added: " << senderAddr << " (dist: " << (int)distance << "m)" << endl;
        }
    }
}

void FANETApp::processSensorData(cPacket *packet, L3Address senderAddr) {
    if (!isGCS) return;
    
    int sourceUAV = packet->par("sourceUAV");
    dataPacketsReceived++;
    emit(dataPacketsReceivedSignal, dataPacketsReceived);
    
    EV << "Sensor data received: UAV" << sourceUAV << " → GCS" << endl;
}

void FANETApp::processDataRelay(cPacket *packet, L3Address senderAddr) {
    int hopCount = packet->par("hopCount");
    
    if (hopCount > MAX_HOP_COUNT) {
        EV << "Max hop count exceeded, dropping packet" << endl;
        return;
    }
    
    if (isGCS) {
        // Destino final alcançado
        processSensorData(packet, senderAddr);
    } else {
        // Continuar relay
        L3Address gcsAddr = findDirectGCSConnection();
        if (!gcsAddr.isUnspecified()) {
            packet->par("hopCount") = hopCount + 1;
            
            Packet *relayPacket = new Packet("FANETDataRelay");
            relayPacket->insertAtBack(makeShared<cPacketChunk>(packet->dup()));
            socket.sendTo(relayPacket, gcsAddr, destPort);
            
            packetsSent++;
            emit(packetsSentSignal, packetsSent);
        }
    }
}

bool FANETApp::parsePositionInfo(const std::string& posStr, Coord& pos, bool& isGCSNode) {
    std::istringstream ss(posStr);
    std::string token;
    std::vector<std::string> tokens;
    
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 4) {
        pos.x = std::stod(tokens[0]);
        pos.y = std::stod(tokens[1]);
        pos.z = std::stod(tokens[2]);
        isGCSNode = (tokens[3] == "GCS");
        return true;
    }
    
    return false;
}

void FANETApp::updateNeighborInfo(L3Address address, Coord position, bool neighborIsGCS) {
    Coord myPos = mobility->getCurrentPosition();
    double distance = calculateDistance(myPos, position);
    
    NeighborInfo info;
    info.address = address;
    info.position = position;
    info.lastSeen = simTime();
    info.isGCS = neighborIsGCS;
    info.distance = distance;
    
    neighbors[address] = info;
}

void FANETApp::socketErrorArrived(UdpSocket *socket, Indication *indication) {
    delete indication;
}

void FANETApp::socketClosed(UdpSocket *socket) {
    // Socket fechado
}

// =============================================================================
// FINALIZAÇÃO COM ESTATÍSTICAS
// =============================================================================
// LIFECYCLE OPERATIONS (REQUIRED BY INET)
// =============================================================================
void FANETApp::handleStartOperation(LifecycleOperation *operation) {
    EV << "FANET application starting" << endl;
    // A configuração já é feita no initialize(), não precisa fazer nada aqui
}

void FANETApp::handleStopOperation(LifecycleOperation *operation) {
    EV << "FANET application stopping" << endl;
    forceFinalization();
}

void FANETApp::handleCrashOperation(LifecycleOperation *operation) {
    EV << "FANET application crashed" << endl;
    forceFinalization();
}

// =============================================================================
void FANETApp::finish() {
    EV << "FANET FINAL STATS - " << (isGCS ? "GCS" : "UAV") << " " << getIndex() 
       << ": sent=" << packetsSent << " recv=" << packetsReceived 
       << " data_sent=" << dataPacketsSent << " data_recv=" << dataPacketsReceived 
       << " neighbors=" << neighbors.size() << endl;
    
    // Registrar estatísticas finais
    if (!isGCS) {
        recordScalar("totalPacketsSent", packetsSent);
        recordScalar("totalPacketsReceived", packetsReceived);
        recordScalar("dataPacketsSent", dataPacketsSent);
        recordScalar("relayAttempts", relayAttempts);
        recordScalar("neighborsFound", (int)neighbors.size());
        
        double deliveryRatio = dataPacketsSent > 0 ? (double)dataPacketsReceived / dataPacketsSent : 0.0;
        recordScalar("dataDeliveryRatio", deliveryRatio);
    } else {
        recordScalar("totalDataReceived", dataPacketsReceived);
        recordScalar("totalNeighbors", (int)neighbors.size());
    }
    
    ApplicationBase::finish();
}