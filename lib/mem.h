#ifndef H_MEM
#define H_MEM

#define _calloc(elements, type) my_calloc(elements, sizeof(type), __FILE__, __LINE__)
#define _realloc(p, elements, type) my_realloc(p, sizeof(type), elements, __FILE__, __LINE__)
#define _free(p) my_free(p, __FILE__, __LINE__)


struct allocations {
	char * location;
	void * pointer;
};

extern unsigned int allocs;

void init_allocs();
void * my_calloc(int elements, int size, const char * file, const int line);
void * my_realloc(void * p, int type_size, int elements, const char * file, const int line);
void * my_free(void * p, const char * file, const int line);
void print_allocs();

#endif