    //
    // Aircraft.ned
    // 
    // Definição do módulo UAV (Veículo Aéreo Não Tripulado)
    // para simulação FANET
    //

    package fanet_simples.src;

    import inet.node.inet.AdhocHost;  // MUDANÇA: AdhocHost para wireless ad-hoc

    //
    // UAV (Unmanned Aerial Vehicle) para rede FANET
    // 
    // Características:
    // - Mobilidade aérea com velocidade configurável
    // - Aplicação FANET para comunicação
    // - Interface wireless IEEE 802.11 em modo ad-hoc
    // - Alcance de transmissão de 200m
    // - Usa AdhocHost para garantir roteamento correto via WLAN
    //
module Aircraft extends AdhocHost  // MUDANÇA AQUI: AdhocHost em vez de StandardHost
{
    parameters:
        @display("i=misc/drone;r=300");  // Removido arranjo automático
        
        // === APLICAÇÃO ===
        numApps = 1;
        app[0].typename = "FANETApp";
        app[0].isGCS = false;
        app[0].maxTransmissionRange = 300m;  // Alcance ajustado para configuração atual
        
        // === PARÂMETROS ESPECÍFICOS ===
        double uavSpeed @unit(mps) = default(20mps);
}