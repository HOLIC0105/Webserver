#if !defined(THREADPOOL_H)
#define THREADPOOL_H

#include <list>
#include "locker.h"
#include <exception>
#include <cstdio>
#include <pthread.h>

//threadpool T is task class
template<typename T> 
class Threadpool{
  public:
    Threadpool(int threadnum = 8, int maxrequests = 10000);
    ~Threadpool();
    bool Append(T request);

  private:
    void run();
    static void * worker(void *arg);
  
  private:
    constexpr int threadnum_;
    pthread_t * threads_; 
    int maxrequests_;
    locker queuelock_;
    sem queuestat_;
    bool stop_;
    std::list<T> workqueue;
};

template<typename T>
Threadpool<T>::Threadpool(int threadnum, int maxrequests) : 
  threadnum_(threadnum), maxrequests_(maxrequests),
  stop_(false), threads_(NULL) {

    if(threadnum_ <= || maxrequests_ <= 0) {
      throw std::exception();
    }

    threads_ = new pthread_t[threadnum_];
    if(!threads_) {
      throw std::exception();
    }

    for(int i = 0; i < threadnum_; ++ i) {
      printf("create  the %dth thread\n", i);

      if(pthread_create(threads + i, NULL, worker, this) != 0) {
        delete[] threads_;
        throw std::exception();
      }

      if(pthread_detach(threads_[i]) != 0) {
        delete [] thread_;
        throw std::exception();
      }

    }
}

template<typename T>
Threadpool<T>:: ~Threadpool(){
  delete [] thread_;
  stop_ = true;
}

template<typename T>
bool Threadpool<T> :: Append(T request) {
  queuelock_.lock();
  if(workqueue.size() > maxrequests_) {
    queuelocker.unlock();
    return false;
  }
  workqueue.push_back(request);
  queuelock_.unlock();
  queuestat.post();
  return true;
}

template<typename T>
void * Threadpool<T> :: worker(void * arg) {
  Threadpool * pool = reinterpret_cast<Threadpool *>(arg);
  pool->run();
  return NULL;
}

template<typename T>
void Threadpool<T> ::run() {
  while(!m_stop) {
    queuestat_.Wait();
    queuelocker.Lock();
    if(workqueue.empty()) {
      queuelocker.Unlock();
      queuestat_.Post();
      continue;
    }

    T request = workqueue.front();
    workqueue.pop_front();
    queuelocker.Unlock();
    if(!request) continue;
    request->process();

  }
}

#endif // THREADPOOL_H
