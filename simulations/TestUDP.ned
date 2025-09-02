//
// TestUDP.ned
// 
// Configuração de teste básico para verificar UDP funcionando
//

package fanet_simples.simulations;

import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;

network TestUDPNetwork
{
    parameters:
        @display("bgb=2000,2000");
        
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            @display("p=100,50");
        }
        
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100");
        }
        
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,150");
        }
        
        sender: AdhocHost {
            @display("p=1000,1000");
        }
        
        receiver: AdhocHost {
            @display("p=1100,1100");
        }
}
