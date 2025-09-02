//
// ArbitraryMobility.ned
// Definição do módulo de mobilidade customizada
//

package fanet_simples.src;

import inet.mobility.contract.IMobility;

//
// Custom mobility module for UAVs with realistic flight patterns
//
simple ArbitraryMobility like IMobility
{
    parameters:
        @class(ArbitraryMobility);
        @signal[mobilityStateChanged](type=inet::MobilityBase);
        @display("i=block/cogwheel");
        
        // Base mobility parameters (inherited from MobilityBase)
        string subjectModule = default("^");
        string coordinateSystemModule = default("");
        string displayStringTextFormat @mutable = default("p: %p\nv: %v");
        bool updateDisplayString = default(true);
        
        // MovingMobilityBase parameters
        bool faceForward = default(true);
        
        // Initial position parameters
        double initialX @unit(m) = default(0m);
        double initialY @unit(m) = default(0m);
        double initialZ @unit(m) = default(0m);
        
        // Parâmetros de velocidade
        double minSpeed @unit(mps) = default(15mps);  // 54 km/h
        double maxSpeed @unit(mps) = default(25mps);  // 90 km/h
        
        // Parâmetros de altitude
        double minAltitude @unit(m) = default(50m);
        double maxAltitude @unit(m) = default(250m);
        
        // Área de contenção
        double constraintAreaMinX @unit(m) = default(100m);
        double constraintAreaMinY @unit(m) = default(100m);
        double constraintAreaMinZ @unit(m) = default(50m);  // Altitude mínima
        double constraintAreaMaxX @unit(m) = default(1900m);
        double constraintAreaMaxY @unit(m) = default(1900m);
        double constraintAreaMaxZ @unit(m) = default(250m); // Altitude máxima
        
        // Parâmetros padrão do IMobility
        double updateInterval @unit(s) = default(0.1s);
}
