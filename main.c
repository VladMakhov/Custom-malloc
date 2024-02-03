#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define STACK_SIZE 32
#define HEAP_SIZE STACK_SIZE * 4
#define HEADER 4
#define LINE_START "_________________________________________________________________________\n"
#define LINE_END "|________________________________________________________________________\n"

static u16 IN_USE; 


typedef struct virtual_memory{
    u8 stack[STACK_SIZE];
    u8 heap[HEAP_SIZE];
    char** unmapped;

    struct 
    {
        char** data;
        char** bss;
        char* text;
    } data_t;
} virtual_memory_t;

typedef struct entity {
    u8* ptr;
    u16 size;
} entity_t;

entity_t LIST[40];

void LOG() {
    printf(LINE_START);
    for (unsigned i = 0; i < IN_USE; i++)
    {
        printf("| Data + HEADER: [%p]. Memory of our heap free: [%u]\n", LIST[i].ptr, LIST[i].size);
    }
    printf("| Entities in use: [%ld]\n", sizeof(LIST) / sizeof(LIST[0]) - IN_USE);
    printf(LINE_END);
}

entity_t* new_entity(size_t size) {
    if (LIST[0].ptr == NULL && LIST[0].size == 0) {
        static virtual_memory_t vm;
        LIST[0].ptr = vm.heap;
        LIST[0].size = HEAP_SIZE;
        IN_USE++;
        LOG();
    }

    entity_t* best = LIST;

    for (unsigned i = 0; i < IN_USE; i++)  {
        if (LIST[i].size >= size && LIST[i].size < best->size) {
            best = &LIST[i];
        }
    }

    return best;
}

void* my_malloc(size_t size) {
    assert(size <= HEAP_SIZE);

    size += HEADER;

    entity_t* e = new_entity(size);

    u8* start = e->ptr;
    u8* user_ptr = start + HEADER;

    *start = size;

    e->ptr += size;
    e->size -= size;

    LOG();

    return user_ptr;
}

void my_free(void* ptr) {
    u8* start = (u8*)ptr - HEADER;

    LIST[IN_USE].ptr = &(*start);
    LIST[IN_USE].size = (u8)*((u8*)ptr - HEADER);
    IN_USE++;
    
    LOG();
}

int main(int argc, char** argv) {
    typedef struct foo {
        int a;
        int b;
    } foo_t;

    foo_t* foo;
    char* bar;
    int* bazz;
    
    foo = my_malloc(sizeof(foo_t));
    bar = my_malloc(5);
    bazz = my_malloc(sizeof(int));

    foo->a = 5;
    foo->b = 10;

    strcpy(bar, "bar");

    memcpy(bazz, &foo->a, sizeof(int));

    printf(LINE_START);
    printf("| Address: [%p], data: [%d] [%d]\n", foo, foo->a, foo->b);
    printf("| Address: [%p], data: [%s]\n", bar, bar);
    printf("| Address: [%p], data: [%d]\n", bazz, *bazz);
    printf(LINE_END);

    my_free(foo);
    my_free(bar);

    return 0;
}