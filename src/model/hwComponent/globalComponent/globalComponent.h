//
// Created by tanawin on 27/1/2567.
//

#ifndef KATHRYN_GLOBALCOMPONENT_H
#define KATHRYN_GLOBALCOMPONENT_H

#include "model/hwComponent/wire/wire.h"
#include "model/flowBlock/abstract/nodes/startNode.h"

/**
 * @file globalComponent.h
 * @brief Contains global component declarations used throughout the Kathryn project
 */

namespace kathryn{
    /**
     * @namespace kathryn
     * @brief Main namespace containing global components and utilities
     */

    /** Global reset wire pointer */
    extern Wire* rstWire;
    /** Global start node pointer */
    extern StartNode* startNode;

    /**
     * @brief Initializes all global components
     * Creates and configures reset wire and start node
     */
    void initiateGlobalComponent();

    /**
     * @brief Resets all global component pointers to null
     * Note: Does not delete objects as they are managed by module destructor
     */
    void resetGlobalComponent();

    /**
     * @brief Gets the global reset signal
     * @return Reference to the global reset wire as Operable
     */
    Operable& getResetSignal();


}

#endif //KATHRYN_GLOBALCOMPONENT_H
