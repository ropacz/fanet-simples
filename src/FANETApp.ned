//
// FANETApp.ned
// 
// Definição da aplicação FANET para comunicação
// entre UAVs e Estação de Controle Terrestre (GCS)
//

package fanet_simples.src;

import inet.applications.contract.IApp;

//
// Aplicação FANET (Flying Ad-hoc Network)
// 
// Esta aplicação implementa:
// - Descoberta automática de vizinhos via broadcast UDP
// - Transmissão de dados de sensores dos UAVs para GCS
// - Relay de dados através de UAVs intermediários
// - Monitoramento de conectividade da rede
//
simple FANETApp like IApp
{
    parameters:
        @class(FANETApp);
        @display("i=block/app");
        
        // === CONFIGURAÇÃO DE REDE ===
        int localPort = default(1000);           // Porta UDP local
        int destPort = default(1000);            // Porta UDP de destino
        double startTime @unit(s) = default(1s); // Tempo de início da aplicação
        
        // === CONFIGURAÇÃO FANET ===
        double neighborTimeout @unit(s) = default(30s);        // Timeout para vizinhos
        double maxTransmissionRange @unit(m) = default(300m);  // Alcance máximo
        bool isGCS = default(false);                           // Se é uma GCS
        
        // === SINAIS PARA ESTATÍSTICAS ===
        @signal[packetsReceived](type=long);
        @signal[packetsSent](type=long);
        @signal[dataPacketsReceived](type=long);
        @signal[dataPacketsSent](type=long);
        @signal[neighborsFound](type=long);
        
        // === ESTATÍSTICAS ===
        @statistic[packetsReceived](title="Packets Received"; source=packetsReceived; record=vector,count,sum);
        @statistic[packetsSent](title="Packets Sent"; source=packetsSent; record=vector,count,sum);
        @statistic[dataPacketsReceived](title="Data Packets Received"; source=dataPacketsReceived; record=vector,count,sum);
        @statistic[dataPacketsSent](title="Data Packets Sent"; source=dataPacketsSent; record=vector,count,sum);
        @statistic[neighborsFound](title="Neighbors Found"; source=neighborsFound; record=vector,count,sum);
        
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}
