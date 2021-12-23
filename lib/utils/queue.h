#ifndef _INCLUDE_UTILS_QUEUE
#define _INCLUDE_UTILS_QUEUE

#include <thread>
#include <mutex>
#include <semaphore.h>

template <class T>
class Queue
{
  public:
    Queue(int MaxSize);
    ~Queue(void);

    bool Add(const T &Item);
    bool AddOnce(const T &Item);

    T    Get(void);
    T    waitGet(void);
    T    waitGet(int timeout);

    bool contains(T);

    inline int getItems(void);
    inline void clear(void);

  private:
    void Insert(const T &Item);
    T    Acquire(void);

  protected:
    std::mutex mutex;
    sem_t semaphore;

    T *Data;
    const int MAX;
    int Beginning = 0, End = 0;

    int Items = 0;
};

template <class T>
Queue<T>::Queue(int MaxSize): MAX(MaxSize) {
  Data = new T[MAX + 1];
  sem_init(&semaphore, 0, 0);
}

template <class T>
Queue<T>::~Queue(){
  delete [] Data;
  sem_destroy(&semaphore);
}

template <class T>
bool Queue<T>::Add(const T &Item){
  const std::lock_guard<std::mutex> lock(mutex);

  if(Items >= MAX || Item == nullptr)
    return false;

  Insert(Item);
  return true;
}

template <class T>
bool Queue<T>::AddOnce(const T &Item){
  const std::lock_guard<std::mutex> lock(mutex);

  if(Items >= MAX || Item == nullptr)
    return false;

  for(int i = 0; i < Items; i++){
    if(Data[(Beginning + i) % MAX] == Item)
      return false;
  }

  Insert(Item);
  return true;
}

template <class T>
void Queue<T>::Insert(const T &Item){
  Data[End++] = Item;
  ++Items;

  if(End > MAX)
    End -= (MAX + 1);

  sem_post(&semaphore);
}

template <class T>
T Queue<T>::Get(void){
  if(Items <= 0)
    return 0;

  if(sem_wait(&semaphore) < 0)
    return 0; //Failed to acquire semaphore

  return Acquire();
}

template <class T>
T Queue<T>::waitGet(void){
  return waitGet(15);
}

template <class T>
T Queue<T>::waitGet(int timeout){
  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);
  t.tv_sec += timeout;

  if(sem_timedwait(&semaphore, &t) < 0){
    return 0; // Failed to acquire semaphore
  }

  return Acquire();
}

template <class T>
T Queue<T>::Acquire(void){
  if(Items <= 0)
    return 0;

  T val = Data[Beginning++];
  --Items;

  if(Beginning > MAX)
    Beginning -= (MAX + 1);

  return val;
}

template <class T>
bool Queue<T>::contains(T Item){
  for(uint16_t i = Beginning; i != End; ){
    
    if(Data[i] == Item)
      return true;

    if(++i > MAX)
      i -= (MAX + 1);
  }

  return false;
}

template <class T>
inline int Queue<T>::getItems(void){
  return Items;
}

template <class T>
inline void Queue<T>::clear(void){
  while(Items > 0)
    Get();
}

#endif
