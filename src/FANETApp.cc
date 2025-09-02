/*
 * FANETApp.cc
 * 
 * Implementação da aplicação FANET (Flying Ad-hoc Network)
 * 
 * Esta aplicação implementa um protocolo de comunicação para redes
 * ad-hoc de veículos aéreos não tripulados (UAVs) e estação de controle
 * terrestre (GCS).
 * 
 * Fluxo principal:
 * 1. Descoberta de vizinhos via broadcast UDP
 * 2. Transmissão de dados de sensores dos UAVs para GCS
 * 3. Relay de dados através de UAVs intermediários quando necessário
 * 4. Monitoramento contínuo de conectividade da rede
 */

#include "FANETApp.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/cPacketChunk.h"
#include <sstream>

using namespace omnetpp;
using namespace inet;

Define_Module(FANETApp);

// =============================================================================
// LIFECYCLE E INICIALIZAÇÃO
// =============================================================================

FANETApp::~FANETApp() {
    cancelAndDelete(neighborDiscoveryTimer);
    cancelAndDelete(dataTransmissionTimer);
    cancelAndDelete(connectivityCheckTimer);
}

void FANETApp::initialize(int stage) {
    ApplicationBase::initialize(stage);
    
    if (stage == INITSTAGE_LOCAL) {
        EV << "FANET Init: " << (par("isGCS").boolValue() ? "GCS" : "UAV") 
           << " " << getIndex() << endl;
        
        // Leitura de parâmetros
        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        neighborTimeout = par("neighborTimeout");
        maxTransmissionRange = par("maxTransmissionRange");
        isGCS = par("isGCS");
        
        // Log dos parâmetros carregados
        EV << "FANET Config: " << (isGCS ? "GCS" : "UAV") << getIndex() 
           << " maxRange=" << maxTransmissionRange << "m" 
           << " port=" << localPort << endl;
        
        // Inicialização de estatísticas
        packetsReceived = 0;
        packetsSent = 0;
        dataPacketsReceived = 0;
        dataPacketsSent = 0;
        
        // Registro de sinais
        packetsReceivedSignal = registerSignal("packetsReceived");
        packetsSentSignal = registerSignal("packetsSent");
        dataPacketsReceivedSignal = registerSignal("dataPacketsReceived");
        dataPacketsSentSignal = registerSignal("dataPacketsSent");
        neighborsFoundSignal = registerSignal("neighborsFound");
        
        // Criação de timers
        neighborDiscoveryTimer = new cMessage("neighborDiscoveryTimer");
        dataTransmissionTimer = new cMessage("dataTransmissionTimer");
        connectivityCheckTimer = new cMessage("connectivityCheckTimer");
        
        // Obter referência para módulo de mobilidade
        mobility = check_and_cast<IMobility *>(getModuleByPath("^.mobility"));
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        configureSocket();
        scheduleTimers();
        
        EV << "FANET ready: " << (isGCS ? "GCS" : "UAV") << " " << getIndex() 
           << " at " << mobility->getCurrentPosition() << endl;
    }
}

void FANETApp::configureSocket() {
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    socket.setBroadcast(true);
    socket.setCallback(this);
    
    EV << "Socket configured on port " << localPort << " with broadcast enabled" << endl;
}

void FANETApp::scheduleTimers() {
    // Descoberta de vizinhos a cada 10 segundos
    scheduleAt(startTime + uniform(1, 3), neighborDiscoveryTimer);
    
    // Transmissão de dados apenas para UAVs (a cada 15 segundos)
    if (!isGCS) {
        scheduleAt(startTime + uniform(5, 8), dataTransmissionTimer);
    }
    
    // Verificação de conectividade a cada 30 segundos
    scheduleAt(startTime + uniform(10, 15), connectivityCheckTimer);
}

// =============================================================================
// GERENCIAMENTO DE MENSAGENS
// =============================================================================

void FANETApp::handleMessageWhenUp(cMessage *msg) {
    if (msg == neighborDiscoveryTimer) {
        sendNeighborDiscovery();
        scheduleAt(simTime() + 10, neighborDiscoveryTimer);  // Repetir a cada 10s
    }
    else if (msg == dataTransmissionTimer) {
        sendSensorData();
        scheduleAt(simTime() + 15, dataTransmissionTimer);   // Repetir a cada 15s
    }
    else if (msg == connectivityCheckTimer) {
        checkConnectivity();
        scheduleAt(simTime() + 30, connectivityCheckTimer); // Repetir a cada 30s
    }
    else if (msg->isSelfMessage()) {
        EV << "Unknown self-message: " << msg->getName() << endl;
        delete msg;
    }
    else {
        // Processamento via UDP socket
        socket.processMessage(msg);
    }
}

// =============================================================================
// FUNCIONALIDADES CORE FANET
// =============================================================================

void FANETApp::sendNeighborDiscovery() {
    Coord myPosition = mobility->getCurrentPosition();
    
    EV << "Starting neighbor discovery: " << (isGCS ? "GCS" : "UAV") << getIndex() 
       << " @ (" << (int)myPosition.x << "," << (int)myPosition.y << "," << (int)myPosition.z << ")" << endl;
    
    // Criar mensagem de descoberta
    cPacket *packet = createFANETMessage(NEIGHBOR_DISCOVERY, "NEIGHBOR_DISCOVERY");
    
    // Adicionar informações de posição
    char posInfo[100];
    sprintf(posInfo, "%.2f,%.2f,%.2f,%s", 
            myPosition.x, myPosition.y, myPosition.z, 
            isGCS ? "GCS" : "UAV");
    packet->addPar("positionInfo") = posInfo;
    
    // Enviar via broadcast
    Packet *discoveryPacket = new Packet("FANETDiscovery");
    discoveryPacket->insertAtBack(makeShared<cPacketChunk>(packet));
    
    EV << "Broadcasting discovery to 255.255.255.255:" << destPort << endl;
    socket.sendTo(discoveryPacket, Ipv4Address::ALLONES_ADDRESS, destPort);
    
    packetsSent++;
    emit(packetsSentSignal, packetsSent);
    
    EV << "Discovery sent: " << (isGCS ? "GCS" : "UAV") << " " << getIndex() 
       << " @ (" << (int)myPosition.x << "," << (int)myPosition.y << ")" << endl;
}

void FANETApp::sendSensorData() {
    // Apenas UAVs enviam dados de sensores
    if (isGCS) return;
    
    Coord myPos = mobility->getCurrentPosition();
    
    // Criar dados simulados de sensores
    char sensorData[200];
    sprintf(sensorData, "UAV_%d_SENSOR: Pos(%.1f,%.1f,%.1f) Temp:%.1f Bat:%.0f%% T:%.2f", 
            getIndex(), myPos.x, myPos.y, myPos.z, 
            uniform(20, 35), uniform(60, 100), simTime().dbl());
    
    L3Address gcsAddr = findGCSAddress();
    
    // Verificar se GCS está realmente no alcance (dupla verificação)
    bool canReachGCS = false;
    if (!gcsAddr.isUnspecified()) {
        for (const auto& neighbor : neighbors) {
            if (neighbor.first == gcsAddr && neighbor.second.distance <= maxTransmissionRange) {
                canReachGCS = true;
                break;
            }
        }
    }
    
    if (canReachGCS) {
        // Envio direto para GCS
        cPacket *packet = createFANETMessage(SENSOR_DATA, "SENSOR_DATA");
        packet->addPar("data") = sensorData;
        packet->addPar("sourceUAV") = getIndex();
        
        Packet *sensorPacket = new Packet("FANETSensorData");
        sensorPacket->insertAtBack(makeShared<cPacketChunk>(packet));
        socket.sendTo(sensorPacket, gcsAddr, destPort);
        
        dataPacketsSent++;
        emit(dataPacketsSentSignal, dataPacketsSent);
        
        EV << "Sensor data sent: UAV" << getIndex() << " → GCS (direct)" << endl;
    }
    else {
        // Usar relay quando GCS não está no alcance
        L3Address relayAddr = findBestRelayToGCS();
        
        if (!relayAddr.isUnspecified()) {
            cPacket *packet = createFANETMessage(DATA_RELAY, "DATA_RELAY");
            packet->addPar("data") = sensorData;
            packet->addPar("sourceUAV") = getIndex();
            packet->addPar("finalDestination") = "GCS";
            packet->addPar("hopCount") = 1;
            
            sendDataRelay(packet, relayAddr);
            
            EV << "Sensor data relayed: UAV" << getIndex() << " → " << relayAddr 
               << " → GCS (no direct path)" << endl;
        }
        else {
            EV << "WARNING: No path to GCS for sensor data (UAV" << getIndex() << ")" << endl;
        }
    }
}

void FANETApp::sendDataRelay(cPacket *originalPacket, L3Address nextHop) {
    // Incrementar contador de hops
    int hopCount = originalPacket->par("hopCount");
    originalPacket->par("hopCount") = hopCount + 1;
    
    // Verificar se não excedeu limite de hops (evitar loops)
    if (hopCount > 5) {
        EV << "WARNING: Max hop count exceeded, dropping packet" << endl;
        delete originalPacket;
        return;
    }
    
    // Reenviar pacote
    Packet *relayPacket = new Packet("FANETDataRelay");
    relayPacket->insertAtBack(makeShared<cPacketChunk>(originalPacket));
    socket.sendTo(relayPacket, nextHop, destPort);
    
    packetsSent++;
    emit(packetsSentSignal, packetsSent);
    
    EV << "Data relayed to " << nextHop << " (hop " << hopCount << ")" << endl;
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
    
    EV << "Connectivity: " << (isGCS ? "GCS" : "UAV") << " " << getIndex() 
       << " has " << totalNeighbors << " neighbors (" << uavNeighbors << " UAVs, " 
       << (gcsConnected ? "GCS connected" : "no GCS") << ")" << endl;
    
    // Para UAVs sem conexão direta com GCS, mostrar relay disponível
    if (!isGCS && gcsConnected == 0 && uavNeighbors > 0) {
        L3Address bestRelay = findBestRelayToGCS();
        if (!bestRelay.isUnspecified()) {
            EV << "  Relay available: " << bestRelay << endl;
        }
    }
    
    emit(neighborsFoundSignal, totalNeighbors);
}

// =============================================================================
// PROCESSAMENTO DE MENSAGENS RECEBIDAS
// =============================================================================

void FANETApp::processNeighborDiscovery(cPacket *packet, L3Address senderAddr) {
    // Extrair informações de posição
    std::string posStr = packet->par("positionInfo").stringValue();
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(posStr);
    
    while (std::getline(tokenStream, token, ',')) {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 4) {
        double x = std::stod(tokens[0]);
        double y = std::stod(tokens[1]);
        double z = std::stod(tokens[2]);
        bool senderIsGCS = (tokens[3] == "GCS");
        
        Coord senderPos(x, y, z);
        Coord myPos = mobility->getCurrentPosition();
        double distance = calculateDistance(myPos, senderPos);
        
        // Log detalhado da verificação de distância
        EV << "Distance check: " << (isGCS ? "GCS" : "UAV") << getIndex() 
           << " @ (" << (int)myPos.x << "," << (int)myPos.y << "," << (int)myPos.z << ")"
           << " ← " << (senderIsGCS ? "GCS" : "UAV") << " @ (" << (int)x << "," << (int)y << "," << (int)z << ")"
           << " = " << (int)distance << "m (max: " << (int)maxTransmissionRange << "m)" << endl;
        
        // Verificar se está no alcance
        if (distance <= maxTransmissionRange) {
            updateNeighborInfo(senderAddr, senderPos, senderIsGCS);
            
            // Enviar resposta
            cPacket *response = createFANETMessage(NEIGHBOR_RESPONSE, "NEIGHBOR_RESPONSE");
                
                char myPosInfo[100];
                sprintf(myPosInfo, "%.2f,%.2f,%.2f,%s", 
                        myPos.x, myPos.y, myPos.z, isGCS ? "GCS" : "UAV");
                response->addPar("positionInfo") = myPosInfo;
                
                Packet *responsePacket = new Packet("FANETResponse");
                responsePacket->insertAtBack(makeShared<cPacketChunk>(response));
                socket.sendTo(responsePacket, senderAddr, destPort);
                
                packetsSent++;
                emit(packetsSentSignal, packetsSent);
            
            EV << "Discovery response: " << (isGCS ? "GCS" : "UAV") << " " << getIndex() 
               << " → " << (senderIsGCS ? "GCS" : "UAV") << " " << senderAddr 
               << " (dist: " << (int)distance << "m)" << endl;
        } else {
            EV << "Discovery ignored: " << senderAddr << " out of range (" 
               << (int)distance << "m > " << (int)maxTransmissionRange << "m)" << endl;
        }
    }
}

void FANETApp::processNeighborResponse(cPacket *packet, L3Address senderAddr) {
    // Extrair informações de posição
    std::string posStr = packet->par("positionInfo").stringValue();
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(posStr);
    
    while (std::getline(tokenStream, token, ',')) {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 4) {
        double x = std::stod(tokens[0]);
        double y = std::stod(tokens[1]);
        double z = std::stod(tokens[2]);
        bool senderIsGCS = (tokens[3] == "GCS");
        
        Coord senderPos(x, y, z);
        Coord myPos = mobility->getCurrentPosition();
        double distance = calculateDistance(myPos, senderPos);
        
        // VERIFICAÇÃO DE DISTÂNCIA TAMBÉM NO RESPONSE
        if (distance <= maxTransmissionRange) {
            updateNeighborInfo(senderAddr, senderPos, senderIsGCS);
            
            EV << "Response processed: " << (senderIsGCS ? "GCS" : "UAV") << " " << senderAddr 
               << " added as neighbor (dist: " << (int)distance << "m)" << endl;
        } else {
            EV << "Response ignored: " << senderAddr << " out of range (" 
               << (int)distance << "m > " << (int)maxTransmissionRange << "m)" << endl;
        }
    }
}

void FANETApp::processSensorData(cPacket *packet, L3Address senderAddr) {
    // Apenas GCS processa dados de sensores
    if (!isGCS) return;
    
    std::string data = packet->par("data").stringValue();
    int sourceUAV = packet->par("sourceUAV");
    
    EV << "Sensor data received: UAV" << sourceUAV << " → GCS" << endl;
    
    dataPacketsReceived++;
    emit(dataPacketsReceivedSignal, dataPacketsReceived);
}

void FANETApp::processDataRelay(cPacket *packet, L3Address senderAddr) {
    std::string finalDest = packet->par("finalDestination").stringValue();
    int hopCount = packet->par("hopCount");
    
    // Verificar se é o destino final
    if (finalDest == "GCS" && isGCS) {
        EV << "Data relay reached GCS (hop " << hopCount << ")" << endl;
        processSensorData(packet, senderAddr);
        return;
    }
    
    // Se não é o destino final, continuar relay
    if (finalDest == "GCS") {
        L3Address gcsAddr = findGCSAddress();
        
        if (!gcsAddr.isUnspecified()) {
            // GCS encontrada diretamente
            sendDataRelay(packet, gcsAddr);
        } else {
            // Tentar outro relay
            L3Address nextRelay = findBestRelayToGCS();
            if (!nextRelay.isUnspecified() && nextRelay != senderAddr) {
                sendDataRelay(packet, nextRelay);
            } else {
                EV << "WARNING: Relay failed - no path to GCS" << endl;
            }
        }
    }
}

// =============================================================================
// UTILITÁRIOS E GERENCIAMENTO DE VIZINHOS
// =============================================================================

L3Address FANETApp::findGCSAddress() {
    for (const auto& neighbor : neighbors) {
        if (neighbor.second.isGCS) {
            return neighbor.first;
        }
    }
    return L3Address();  // GCS não encontrada
}

L3Address FANETApp::findBestRelayToGCS() {
    L3Address bestRelay;
    double minDistance = std::numeric_limits<double>::max();
    
    // Procurar UAV mais próximo da GCS
    for (const auto& neighbor : neighbors) {
        if (!neighbor.second.isGCS) {  // Apenas UAVs podem ser relays
            // Aqui poderia implementar lógica mais sofisticada
            // Por enquanto, escolher o UAV mais próximo
            if (neighbor.second.distance < minDistance) {
                minDistance = neighbor.second.distance;
                bestRelay = neighbor.first;
            }
        }
    }
    
    return bestRelay;
}

double FANETApp::calculateDistance(const Coord& pos1, const Coord& pos2) {
    double dx = pos1.x - pos2.x;
    double dy = pos1.y - pos2.y;
    double dz = pos1.z - pos2.z;
    return sqrt(dx*dx + dy*dy + dz*dz);
}

void FANETApp::updateNeighborInfo(L3Address address, Coord position, bool neighborIsGCS) {
    Coord myPos = mobility->getCurrentPosition();
    double distance = calculateDistance(myPos, position);
    
    NeighborInfo info(address, position, simTime(), neighborIsGCS);
    info.distance = distance;
    
    bool isNewNeighbor = (neighbors.find(address) == neighbors.end());
    neighbors[address] = info;
    
    if (isNewNeighbor) {
        EV << "New neighbor: " << (neighborIsGCS ? "GCS" : "UAV") << " " << address 
           << " @ " << (int)distance << "m" << endl;
    }
}

void FANETApp::cleanExpiredNeighbors() {
    simtime_t now = simTime();
    auto it = neighbors.begin();
    int removed = 0;
    
    while (it != neighbors.end()) {
        if (now - it->second.lastSeen > neighborTimeout) {
            it = neighbors.erase(it);
            removed++;
        } else {
            ++it;
        }
    }
    
    if (removed > 0) {
        EV << "Cleaned " << removed << " expired neighbors" << endl;
    }
}

cPacket* FANETApp::createFANETMessage(FANETMessageType type, const char* data) {
    char msgName[50];
    sprintf(msgName, "%s_%d", data ? data : "FANET_MSG", getIndex());
    
    cPacket *packet = new cPacket(msgName);
    packet->addPar("messageType") = type;
    packet->addPar("nodeIndex") = getIndex();
    packet->addPar("timestamp") = simTime().dbl();
    packet->setByteLength(64);  // Tamanho base do cabeçalho FANET
    
    return packet;
}

// =============================================================================
// CALLBACKS UDP SOCKET
// =============================================================================

void FANETApp::socketDataArrived(UdpSocket *socket, Packet *packet) {
    packetsReceived++;
    emit(packetsReceivedSignal, packetsReceived);
    
    // Extrair endereço do remetente
    auto l3AddressTag = packet->getTag<L3AddressInd>();
    L3Address senderAddress = l3AddressTag->getSrcAddress();
    
    // FILTRAR LOOPBACK - Ignorar mensagens próprias
    if (senderAddress.str() == "127.0.0.1" || senderAddress.str() == "localhost") {
        EV << "DROPPED: Ignoring loopback message from " << senderAddress << endl;
        delete packet;
        return;
    }
    
    // Obter próprio endereço IP para comparação
    L3Address myAddress = L3AddressResolver().resolve(getParentModule()->getFullPath().c_str());
    if (senderAddress == myAddress) {
        EV << "DROPPED: Ignoring own broadcast message from " << senderAddress << endl;
        delete packet;
        return;
    }
    
    // Extrair payload FANET para verificação prévia
    auto chunk = packet->peekAtBack<cPacketChunk>(packet->getDataLength());
    if (!chunk) {
        EV << "Invalid packet format from " << senderAddress << endl;
        delete packet;
        return;
    }
    
    cPacket *fanetPacket = chunk->getPacket()->dup();
    
    // VERIFICAÇÃO PRÉVIA DE DISTÂNCIA para todos os tipos de mensagem
    bool hasPositionInfo = fanetPacket->hasPar("positionInfo");
    if (hasPositionInfo) {
        std::string posStr = fanetPacket->par("positionInfo").stringValue();
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(posStr);
        
        while (std::getline(tokenStream, token, ',')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 4) {
            double x = std::stod(tokens[0]);
            double y = std::stod(tokens[1]);
            double z = std::stod(tokens[2]);
            
            Coord senderPos(x, y, z);
            Coord myPos = mobility->getCurrentPosition();
            double distance = calculateDistance(myPos, senderPos);
            
            // Verificação de alcance ANTES do processamento
            if (distance > maxTransmissionRange) {
                EV << "DROPPED: Message from " << senderAddress 
                   << " out of range (" << (int)distance << "m > " 
                   << (int)maxTransmissionRange << "m)" << endl;
                delete fanetPacket;
                delete packet;
                return;
            }
        }
    }
    
    EV << "Message received: " << packet->getName() << " from " << senderAddress << endl;
    
    // Processar baseado no tipo de mensagem
    int messageType = fanetPacket->par("messageType");
    
    switch (messageType) {
        case NEIGHBOR_DISCOVERY:
            processNeighborDiscovery(fanetPacket, senderAddress);
            break;
        case NEIGHBOR_RESPONSE:
            processNeighborResponse(fanetPacket, senderAddress);
            break;
        case SENSOR_DATA:
            processSensorData(fanetPacket, senderAddress);
            break;
        case DATA_RELAY:
            processDataRelay(fanetPacket, senderAddress);
            break;
        default:
            EV << "Unknown message type: " << messageType << endl;
            break;
    }
    
    delete fanetPacket;
    delete packet;
}

void FANETApp::socketErrorArrived(UdpSocket *socket, Indication *indication) {
    EV << "UDP socket error" << endl;
    delete indication;
}

void FANETApp::socketClosed(UdpSocket *socket) {
    EV << "UDP socket closed" << endl;
}

void FANETApp::finish() {
    EV << "FANET Stats - " << (isGCS ? "GCS" : "UAV") << " " << getIndex() 
       << ": sent=" << packetsSent << " recv=" << packetsReceived 
       << " data_sent=" << dataPacketsSent << " data_recv=" << dataPacketsReceived 
       << " neighbors=" << neighbors.size() << endl;
    
    ApplicationBase::finish();
}
