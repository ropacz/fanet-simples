/*
 * ArbitraryMobility.cc
 * 
 * Custom mobility model for FANET UAVs
 * Updated for compatibility with INET 4.5.4 and OMNeT++ 6.2.0
 */

#include "ArbitraryMobility.h"
#include "inet/common/ModuleAccess.h"
#include <cmath>

using namespace omnetpp;
using namespace inet;

Define_Module(ArbitraryMobility);

ArbitraryMobility::ArbitraryMobility()
{
    lastUpdate = 0;
    moveTimer = nullptr;
}

ArbitraryMobility::~ArbitraryMobility()
{
    cancelAndDelete(moveTimer);
}

void ArbitraryMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    
    if (stage == INITSTAGE_LOCAL) {
        // Read boundary parameters
        constraintAreaMinX = par("constraintAreaMinX");
        constraintAreaMaxX = par("constraintAreaMaxX");
        constraintAreaMinY = par("constraintAreaMinY");
        constraintAreaMaxY = par("constraintAreaMaxY");
        minAltitude = par("minAltitude");
        maxAltitude = par("maxAltitude");
        
        // Create movement timer
        moveTimer = new cMessage("moveTimer");
        
        EV << "ArbitraryMobility: Boundary area: X[" << constraintAreaMinX << "," << constraintAreaMaxX 
           << "] Y[" << constraintAreaMinY << "," << constraintAreaMaxY 
           << "] Z[" << minAltitude << "," << maxAltitude << "]" << endl;
    }
    else if (stage == INITSTAGE_LAST) {
        // Schedule first movement update
        double updateInterval = par("updateInterval");
        scheduleAt(simTime() + updateInterval, moveTimer);
    }
}

void ArbitraryMobility::setInitialPosition()
{
    // Read initial position from parameters
    double x = par("initialX");
    double y = par("initialY");
    double z = par("initialZ");
    
    lastPosition = Coord(x, y, z);
    
    // Set initial velocity
    double speed = uniform(par("minSpeed").doubleValue(), par("maxSpeed").doubleValue());
    lastVelocity = Coord(speed, 0, 0);
    
    EV << "ArbitraryMobility: setInitialPosition called with (" << x << ", " << y << ", " << z << ")" << endl;
}

void ArbitraryMobility::move()
{
    simtime_t now = simTime();
    
    if (now > lastUpdate) {
        simtime_t timeDelta = now - lastUpdate;
        
        // Update position: new_position = current_position + velocity * time
        Coord newPosition = lastPosition + lastVelocity * timeDelta.dbl();
        
        // Apply boundary constraints with bounce
        bool bounced = false;
        if (newPosition.x <= constraintAreaMinX) {
            newPosition.x = constraintAreaMinX + 1; // Pequena margem para evitar oscilação
            lastVelocity.x = abs(lastVelocity.x); // Força direção positiva
            bounced = true;
        } else if (newPosition.x >= constraintAreaMaxX) {
            newPosition.x = constraintAreaMaxX - 1;
            lastVelocity.x = -abs(lastVelocity.x); // Força direção negativa
            bounced = true;
        }
        
        if (newPosition.y <= constraintAreaMinY) {
            newPosition.y = constraintAreaMinY + 1;
            lastVelocity.y = abs(lastVelocity.y);
            bounced = true;
        } else if (newPosition.y >= constraintAreaMaxY) {
            newPosition.y = constraintAreaMaxY - 1;
            lastVelocity.y = -abs(lastVelocity.y);
            bounced = true;
        }
        
        if (newPosition.z <= minAltitude) {
            newPosition.z = minAltitude + 1;
            lastVelocity.z = abs(lastVelocity.z);
            bounced = true;
        } else if (newPosition.z >= maxAltitude) {
            newPosition.z = maxAltitude - 1;
            lastVelocity.z = -abs(lastVelocity.z);
            bounced = true;
        }
        
        // Add some randomness to direction (menos frequente se acabou de rebater)
        if (!bounced && uniform(0, 1, 0) < 0.1) { // 10% chance of direction change
            double angle = uniform(0, 2 * M_PI, 0);
            double speed = sqrt(lastVelocity.x * lastVelocity.x + lastVelocity.y * lastVelocity.y);
            lastVelocity.x = speed * cos(angle);
            lastVelocity.y = speed * sin(angle);
        }
        
        if (bounced) {
            EV << "ArbitraryMobility: UAV bounced at position (" << newPosition.x 
               << ", " << newPosition.y << ", " << newPosition.z << ")" << endl;
        }
        
        lastPosition = newPosition;
    }
    
    lastUpdate = now;
    emitMobilityStateChangedSignal();
}

void ArbitraryMobility::orient()
{
    // Basic orientation - can be enhanced later
}

void ArbitraryMobility::handleSelfMessage(cMessage *message)
{
    if (message == moveTimer) {
        // Update position
        move();
        
        // Schedule next update
        double updateInterval = par("updateInterval");
        scheduleAt(simTime() + updateInterval, moveTimer);
    }
    else {
        MovingMobilityBase::handleSelfMessage(message);
    }
}

void ArbitraryMobility::finish()
{
    MovingMobilityBase::finish();
}

void ArbitraryMobility::setPositionVelocity(const Coord& position, const Coord& velocity)
{
    lastPosition = position;
    lastVelocity = velocity;
    emitMobilityStateChangedSignal();
}

double ArbitraryMobility::getMaxSpeed() const
{
    return par("maxSpeed");
}
