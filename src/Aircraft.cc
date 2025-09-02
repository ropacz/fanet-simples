/*
 * Aircraft.cc
 * UAV implementation for FANET simulation
 * Implementa UAV com mobilidade e comunicação FANET
 */

#include "Aircraft.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"

using namespace omnetpp;
using namespace inet;

// Registra o módulo Aircraft no sistema OMNeT++ para instanciação via arquivos .ned
Define_Module(Aircraft);

/**
 * Método de inicialização do módulo Aircraft (UAV)
 * Configura o UAV com mobilidade e comunicação FANET
 * @param stage - estágio de inicialização
 */
void Aircraft::initialize(int stage) {
    cSimpleModule::initialize(stage);
    
    // Primeira fase: configuração básica
    if (stage == INITSTAGE_LOCAL) {
        EV << "Initializing Aircraft UAV[" << getId() << "]" << endl;
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        EV << "Aircraft UAV[" << getId() << "] mobility configured" << endl;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        EV << "Aircraft UAV[" << getId() << "] application layer ready" << endl;
    }
}

/**
 * Método chamado ao final da simulação
 */
void Aircraft::finish() {
    cSimpleModule::finish();
    EV << "Aircraft UAV[" << getId() << "] simulation finished" << endl;
}
