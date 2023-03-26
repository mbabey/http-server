#ifndef MEMORY_MANAGER_MANAGER_H
#define MEMORY_MANAGER_MANAGER_H

#include <stddef.h>

/**
 * struct memory_manager
 * <p>
 * A memory manager. Stores a linked list of memory addresses.
 * </p>
 */
struct memory_manager
{
    struct memory_address *head;
};

/**
 * init_mem_manager
 * <p>
 * Create and initialize a memory manager.
 * </p>
 * @return a memory manager
 */
struct memory_manager *init_mem_manager(void);

/**
 * free_mem_manager
 * <p>
 * Free all memory in a memory manager, then free the memory manager itself.
 * If the memory manager does not exist, set errno to EFAULT.
 * </p>
 * @param mem_manager the memory manager to be freed
 * @return 0 on success, -1 and set errno if the memory manager does not exist
 */
int free_mem_manager(struct memory_manager *mem_manager);

/**
 * mm_add
 * <p>
 * Add a new memory address node to the memory manager linked list.
 * If the memory manager does not exist, set errno to EFAULT.
 * </p>
 * @param mem_manager the memory manager.
 * @param mem - the memory to add.
 * @return - the address of the node added, NULL on failure
 */
void *mm_add(struct memory_manager *mem_manager, void *mem);

/**
 * mm_free
 * <p>
 * Free the parameter memory address and remove it from the memory manager.
 * Return -1 and set errno to ENODATA if the memory address cannot be
 * located in the memory manager.
 * If the memory manager does not exist, set errno to EFAULT.
 * If the memory pointed to by ptr cannot be found in the memory manager, set errno to ENODATA.
 * </p>
 * @param mem_manager - the memory manager to search
 * @param mem - the memory address to free
 * @return 0 on success, -1 and set errno on failure
 */
int mm_free(struct memory_manager *mem_manager, void *mem);

/**
 * mm_free_all
 * <p>
 * Free all memory stored in the memory manager.
 * If the memory manager does not exist, set errno to EFAULT.
 * </p>
 * @return the number of memory items freed on success, -1 and set errno on failure
 */
int mm_free_all(struct memory_manager *mem_manager);

/**
 * mm_malloc
 * <p>
 * Call malloc to allocate memory to a pointer. Add the pointer to a
 * memory manager if provided.
 * If the memory manager does not exist, set errno to EFAULT.
 * </p>
 * @param size the number of bytes of memory to allocate
 * @param mem_manager the memory manager to which to add the new memory
 * @return a pointer to the newly allocated memory, NULL and set errno on failure
 */
void *mm_malloc(size_t size, struct memory_manager *mem_manager);

/**
 * mm_calloc
 * <p>
 * Call calloc to allocate memory to a pointer. Add the pointer to a
 * memory manager if provided.
 * If the memory manager does not exist, set errno to EFAULT.
 * </p>
 * @param count the units of memory to allocate
 * @param size the the size of the units of memory
 * @param mem_manager the memory manager to which to add the new memory
 * @return a pointer to the newly allocated memory, NULL and set errno on failure
 */
void *mm_calloc(size_t count, size_t size, struct memory_manager *mem_manager);

/**
 * mm_realloc
 * <p>
 * Call realloc to reallocate memory to a pointer. Update the pointer in the
 * memory manager if provided.
 * If the memory manager does not exist, set errno to EFAULT.
 * If the memory pointed to by ptr cannot be found in the memory manager, set errno to ENODATA.
 * </p>
 * @param ptr the pointer for which to reallocate memory
 * @param size the new size of the memory
 * @param mem_manager the memory manager to which to add the new memory
 * @return a pointer to the newly allocated memory, or NULL and set errno of failure
 */
void *mm_realloc(void *ptr, size_t size, struct memory_manager *mem_manager);

#endif //MEMORY_MANAGER_MANAGER_H
