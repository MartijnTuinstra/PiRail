#ifndef _INCLUDE_UTILS_QUEUE
#define _INCLUDE_UTILS_QUEUE

template <class T>
class FiFo
{
  public:
    FiFo(int MaxSize);
    ~FiFo(void);

    bool Add(const T &Item);
    bool AddOnce(const T &Item);

    T    Get(void);

    inline int getItems(void);
    inline void clear(void);

  private:
    void Insert(const T &Item);
    T    Acquire(void);

  protected:
    T * Array;
    const int MAX;
    int Beginning, End;

    int Items;
};

template <class T>
Queue<T>::Queue(int MaxSize): MAX(MaxSize){
  Data = new T[MAX + 1];
  Beginning = 0;
  End = 0;
  Items = 0;
}

template <class T>
Queue<T>::~Queue(){
  delete Data;
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
inline int Queue<T>::getItems(void){
  return Items;
}

template <class T>
inline void Queue<T>::clear(void){
  while(Items > 0)
    Get();
}

#endif
