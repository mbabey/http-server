#ifndef PROCESS_SERVER_CORE_H
#define PROCESS_SERVER_CORE_H

#include "objects.h"

/**
 * setup_core_object
 * <p>
 * Parse arguments and set up the core object.
 * </p>
 * @param co the core object
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 * @return 0 on success. On failure, -1 and set err
 */
int setup_core_object(struct core_object *co, int argc, char **argv);

/**
 * destroy_core_object
 * <p>
 * Destroy the core object and all of its fields. Does not destroy the state object;
 * the state object must be destroyed by the library destroy_server function.
 * </p>
 * @param co the core object
 */
void destroy_core_object(struct core_object *co);

#endif //PROCESS_SERVER_CORE_H
