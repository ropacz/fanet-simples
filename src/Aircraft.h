/*
 * Aircraft.h
 * UAV module for FANET simulation
 * Integra mobilidade, comunicação e aplicação FANET
 */

#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/mobility/contract/IMobility.h"

using namespace omnetpp;
using namespace inet;

/**
 * Aircraft - Implementação de UAV para FANET
 * Módulo que representa um UAV com mobilidade e comunicação
 */
class INET_API Aircraft : public cSimpleModule {
  protected:
    // Métodos de inicialização e finalização
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void finish() override;

public:
    Aircraft() {}
    virtual ~Aircraft() {}
};

#endif /* AIRCRAFT_H_ */
