#include "manager.h"
#include <errno.h>
#include <stdlib.h>

/**
 * mm_free_recurse
 * <p>
 * Recursively free memory in the linked list of memory addresses.
 * </p>
 * @param ma the memory address to free
 * @return the number of addresses freed
 */
static int mm_free_recurse(struct memory_address *ma);

/**
 * mm_find_in_list
 * <p>
 * Find the memory address struct holding mem in a memory manager.
 * </p>
 * @param mem_manager the memory manager in which to search
 * @param mem the memory for which to search
 * @return the memory address struct holding mem; NULL if not found
 */
static struct memory_address *mm_find_in_list(struct memory_manager *mem_manager, void *mem);

/**
 * struct memory_address
 * <p>
 * A node in the list of memory addresses.
 * </p>
 */
struct memory_address
{
    void                  *addr;
    struct memory_address *next;
};

struct memory_manager *init_mem_manager(void)
{
    struct memory_manager *mm;
    
    mm = (struct memory_manager *) malloc(sizeof(struct memory_manager));
    if (mm)
    {
        mm->head = NULL;
    }
    
    return mm;
}

int free_mem_manager(struct memory_manager *mem_manager)
{
    if (!mem_manager)
    {
        errno = EFAULT;
        return -1;
    }
    
    mm_free_all(mem_manager);
    free(mem_manager);
    
    return 0;
}

void *mm_add(struct memory_manager *mem_manager, void *mem)
{
    struct memory_address *ma;
    struct memory_address *ma_cur;
    
    errno = 0;
    
    if (!mem_manager)
    {
        errno = EFAULT;
        return NULL;
    }
    
    ma = (struct memory_address *) malloc(sizeof(struct memory_address));
    if (!ma)
    {
        return NULL;
    }
    
    ma->addr = mem;
    ma->next = NULL;
    
    if (mem_manager->head == NULL) // If empty list.
    {
        mem_manager->head = ma;
    } else // If non-empty list.
    {
        ma_cur = mem_manager->head;
        while (ma_cur->next != NULL)
        {
            ma_cur = ma_cur->next;
        }
        ma_cur->next = ma;
    }
    
    return ma->addr;
}

int mm_free(struct memory_manager *mem_manager, void *mem)
{
    struct memory_address *ma;
    struct memory_address *ma_prev;
    
    errno = 0;
    
    if (!mem_manager)
    {
        errno = EFAULT;
        return -1;
    }
    
    ma = mem_manager->head;
    while (ma && ma->addr != mem) // Find ma in list, track ma_prev
    {
        ma_prev = ma;
        ma      = ma->next;
    }
    
    if (!ma) // If not found.
    {
        errno = ENODATA;
        return -1;
    }
    
    if (ma == mem_manager->head) // Remove the memory from the manager.
    {
        mem_manager->head = ma->next;
    } else
    {
        ma_prev->next = ma->next;
    }
    
    free(ma->addr); // Free the memory.
    free(ma);
    
    return 0;
}

int mm_free_all(struct memory_manager *mem_manager)
{
    int m_freed;
    
    m_freed = mm_free_recurse(mem_manager->head);
    
    return m_freed;
}

static int mm_free_recurse(struct memory_address *ma) // NOLINT(misc-no-recursion) : intentional recursion
{
    if (ma == NULL)
    {
        return 0;
    }
    
    int m_freed;
    
    m_freed = 1 + mm_free_recurse(ma->next);
    
    free(ma->addr);
    free(ma);
    
    return m_freed;
}

void *mm_malloc(size_t size, struct memory_manager *mem_manager)
{
    void *mem;
    
    errno = 0;
    
    if (!mem_manager)
    {
        errno = EFAULT;
        return NULL;
    }
    
    mem = malloc(size);
    if (!mem)
    {
        return NULL;
    }
    
    mm_add(mem_manager, mem);
    
    
    return mem;
}

void *mm_calloc(size_t count, size_t size, struct memory_manager *mem_manager)
{
    void *mem;
    
    errno = 0;
    
    if (!mem_manager)
    {
        errno = EFAULT;
        return NULL;
    }
    
    mem = calloc(count, size);
    if (!mem)
    {
        return NULL;
    }
    
    mm_add(mem_manager, mem);
    
    return mem;
}

void *mm_realloc(void *ptr, size_t size, struct memory_manager *mem_manager)
{
    struct memory_address *ma;
    void                  *mem;
    
    errno = 0;
    
    // Find the memory address node in the memory manager.
    if (mem_manager)
    {
        ma = mm_find_in_list(mem_manager, ptr);
        if (ma == NULL)
        {
            errno = ENODATA;
            return NULL; // mem not a part of memory manager.
        }
    } else
    {
        errno = EFAULT;
        return NULL; // No memory manager.
    }
    
    mem = realloc(ptr, size);
    if (!mem)
    {
        return NULL;
    }
    
    ma->addr = mem;
    
    return mem;
}

static struct memory_address *mm_find_in_list(struct memory_manager *mem_manager, void *mem)
{
    struct memory_address *ma = NULL;
    
    ma = mem_manager->head;
    while (ma && ma->addr != mem)
    {
        ma = ma->next;
    }
    
    return ma;
}
