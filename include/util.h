#ifndef MXC_UTIL_H
#define MXC_UTIL_H

#include <stdint.h>

typedef struct Vector {
    void **data;
    uint16_t len;
    uint16_t reserved;
} Vector;

Vector *New_Vector(void);
Vector *New_Vector_With_Size(int);
void Delete_Vector(Vector *);
void vec_push(Vector *self, void *d);
void *vec_pop(Vector *self);
void *vec_last(Vector *self);

typedef struct Map {
    Vector *key;
    Vector *value;
} Map;

Map *New_Map(void);
void map_push(Map *, void *, void *);
void *map_search(Map *, char *);

typedef struct String {
    char *data;
    uint16_t len;
    uint16_t reserved;
} String;

String *New_String(void);
void string_push(String *, char);
char string_pop(String *self);

int get_digit(int);
char *read_file(char *);

#endif
