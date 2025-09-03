//
// GCS.ned
// 
// Definição da Estação de Controle Terrestre (Ground Control Station)
// para simulação FANET
//

package fanet_simples.src;

import inet.node.inet.AdhocHost;  // MUDANÇA CRÍTICA: AdhocHost para wireless ad-hoc

//
// Estação de Controle Terrestre para rede FANET
// 
// Características:
// - Posição fixa no centro da área de simulação
// - Aplicação FANET configurada como GCS
// - Interface wireless com maior alcance (300m)
// - Recebe dados de sensores dos UAVs
// - Usa AdhocHost para garantir roteamento correto via WLAN
//
module GCS extends AdhocHost  // MUDANÇA AQUI: AdhocHost em vez de StandardHost
{
    parameters:
        @display("i=device/antennatower_l;r=400");  // Removido arranjo automático

        // === APLICAÇÃO ===
        numApps = 1;
        app[0].typename = "FANETApp";
        app[0].isGCS = true;
        app[0].maxTransmissionRange = 400m;   // GCS alcance ajustado para configuração atual
        
        // === REDE ===
        // AdhocHost já tem configuração adequada para ad-hoc
        // Não precisar configurar forwarding explicitamente
        
        // === CONFIGURAÇÃO WIRELESS INET 4.5.4 ===
        // AdhocHost já tem configuração wireless padrão adequada
        // Configurações específicas serão feitas no omnetpp.ini
}
