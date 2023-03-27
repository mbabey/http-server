#include "../include/db.h"

#include <ndbm.h>

int db_upsert(struct core_object *co, struct state_object *so, void *key, void *value)
{
    PRINT_STACK_TRACE(co->tracer);
    
    int overwrite_status;
    
    
    
    overwrite_status = 0;
    
    return overwrite_status;
}
