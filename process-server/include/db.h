#ifndef HTTP_SERVER_DB_H
#define HTTP_SERVER_DB_H

#include "objects.h"

/**
 * db_upsert
 * <p>
 * Upsert into the database.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param key the key to upsert
 * @param value the value to upsert
 * @return 0 on success and no overwrite, 1 on success and overwrite, -1 and set err on failure
 */
int db_upsert(struct core_object *co, struct state_object *so, void *key, void *value);

#endif //HTTP_SERVER_DB_H
