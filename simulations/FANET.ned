//
// FANET.ned
// 
// Definição da rede FANET (Flying Ad-hoc Network)
// 
// Esta rede simula um ambiente com UAVs (drones) e uma
// Estação de Controle Terrestre (GCS) comunicando via
// protocolo FANET sobre IEEE 802.11 em modo ad-hoc.
//

package fanet_simples.simulations;

import fanet_simples.src.Aircraft;
import fanet_simples.src.GCS;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.environment.common.PhysicalEnvironment;

//
// Rede FANET - Flying Ad-hoc Network
// 
// Componentes:
// - numUAVs UAVs com mobilidade aérea
// - 1 GCS estacionária no centro
// - Meio de transmissão wireless IEEE 802.11
// - Configurador de rede IPv4 automático
//
network FANET
{
    parameters:
        @display("bgb=2000,2000;bgi=background/terrain,s;bgg=100,2,grey95;bgs=0.075,km");
        @figure[title](type=label; pos=1000,50; anchor=n; color=darkblue; font=Arial,18; text="FANET - Flying Ad-hoc Network");
        
        int numUAVs = default(5);
        
    submodules:
        // === INFRAESTRUTURA DE REDE ===
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,50");
        }
        
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=300,50");
        }
        
        physicalEnvironment: PhysicalEnvironment {
            @display("p=500,50");
        }
        
        // === NÓS DA REDE ===
        // Estação de Controle Terrestre (fixa no centro)
        gcs: GCS {
            @display("p=1000,1000");
        }
        
        // Veículos Aéreos Não Tripulados (posições definidas no omnetpp.ini)
        uav[numUAVs]: Aircraft {
            @display("p=100,100");  // Posição default, será sobrescrita pelo omnetpp.ini
        }
}
