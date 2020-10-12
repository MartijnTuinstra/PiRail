#ifndef _INCLUDE_UTILS_DYNARRAY_H
#define _INCLUDE_UTILS_DYNARRAY_H

#include <stdint.h>
#include <stdio.h>

template <class T>
class dynArray {
  public:
    T * array;
    uint8_t increaseSize = 30;

    void allocate(int);

    uint16_t lastIndex = 0;
    uint16_t size = 0;
    uint16_t items = 0;

    dynArray<T>();
    dynArray<T>(int);
    ~dynArray<T>();

    int  push_back(T&);
    // int  push_back(T);
    void insertAt(T&, uint16_t);

    T operator[](int);
    T at(int);

    void remove(T&);

    void print();

    void clear();
    void empty();
};


template <class T>
dynArray<T>::dynArray(){
  allocate(increaseSize);
}

template <class T>
dynArray<T>::dynArray(int i){
  increaseSize = i;
  allocate(increaseSize);
}

template <class T>
dynArray<T>::~dynArray(){
  _free(array);
  size = 0;
}

template <class T>
inline void dynArray<T>::allocate(int diff){
  if(size == 0){
    size += diff;
    array = (T *)_calloc(size, T);
  }
  else{
    uint16_t oldSize = size;
    size += diff;
    array = (T *)_realloc(array, size, T);
    memset(&array[oldSize], 0, diff * sizeof(T *));
  }
}

template <class T>
inline int dynArray<T>::push_back(T& value){
  if(lastIndex >= size)
    allocate(increaseSize);

  array[lastIndex] = value;
  items++;

  return lastIndex++;
}

// template <class T>
// inline int dynArray<T>::push_back(T value){
//   if(lastIndex >= size)
//     allocate(increaseSize);

//   array[lastIndex] = value;
//   items++;

//   return lastIndex++;
// }

template <class T>
inline void dynArray<T>::insertAt(T& value, uint16_t index){
  if(index >= size)
    allocate(increaseSize);

  array[index] = value;
  items++;

  if(index >= lastIndex)
    lastIndex = index + 1;
}

template <class T>
inline T dynArray<T>::operator[](int index){
  return array[index];
}

template <class T>
inline T dynArray<T>::at(int index){
  return array[index];
}

template <class T>
inline void dynArray<T>::remove(T& ptr){
  for(int i = 0; i < size; i++){
    if(array[i] == ptr){
      array[i] = 0;
      return;
    }
  }
}

template <class T>
void dynArray<T>::print(){
  printf("dynArray(%i items, size %i)\n", items, size);
}

template <class T>
void dynArray<T>::clear(){
  for(int i = 0; i < size;i++){
    if(array[i]){
      delete array[i];
    }
  }

  empty();
}

template <class T>
void dynArray<T>::empty(){
  _free(array);
  size = 0;
  lastIndex = 0;
  items = 0;
  allocate(increaseSize);
}

#endif