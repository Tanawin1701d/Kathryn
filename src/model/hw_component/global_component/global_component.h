//
// Created by tanawin on 27/1/2567.
//

#ifndef KATHRYN_GLOBALCOMPONENT_H
#define KATHRYN_GLOBALCOMPONENT_H

#include "model/hw_component/wire/wire.h"
#include "model/flow_block/abstract/nodes/start_node.h"

/**
 * @file global_component.h
 * @brief Contains global component declarations used throughout the Kathryn project
 */

namespace kathryn{
    /**
     * @namespace kathryn
     * @brief Main namespace containing global components and utilities
     */

    /** Global reset wire pointer */
    extern Wire* rst_wire;
    /** Global start node pointer */
    extern StartNode* start_node;

    /**
     * @brief Initializes all global components
     * Creates and configures reset wire and start node
     */
    void initiate_global_component();

    /**
     * @brief Resets all global component pointers to null
     * Note: Does not delete objects as they are managed by module destructor
     */
    void reset_global_component();

    /**
     * @brief Gets the global reset signal
     * @return Reference to the global reset wire as Operable
     */
    Operable& get_reset_signal();


}

#endif //KATHRYN_GLOBALCOMPONENT_H
