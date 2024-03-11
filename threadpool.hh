#ifndef THREADPOOL_H 
#define THREADPOOL_H

#include <list>
#include "locker.hh"
#include <exception>
#include <cstdio>
#include <pthread.h>

//threadpool T is task class
template<typename T> 
class Threadpool{
  public:
    Threadpool(int threadnum = 8, int maxrequests = 10000);
    ~Threadpool();
    bool Append(T *request);

  private:
    void run();
    static void * worker(void *arg);
  
  private:
    const int threadnum_;
    pthread_t * threads_; 
    int maxrequests_;
    Locker queuelock_;
    Sem queuestat_;
    bool stop_;
    std::list<T *> workqueue_;
};

template<typename T>
Threadpool<T>::Threadpool(int threadnum, int maxrequests) : 
  threadnum_(threadnum), maxrequests_(maxrequests),
  stop_(false), threads_(NULL) {

    if(threadnum_ <= 0|| maxrequests_ <= 0) {
      throw std::exception();
    }

    threads_ = new pthread_t[threadnum_];
    if(!threads_) {
      throw std::exception();
    }

    for(int i = 0; i < threadnum_; ++ i) {
      printf("create  the %dth thread\n", i);

      if(pthread_create(threads_ + i, NULL, worker, this) != 0) {
        delete[] threads_;
        throw std::exception();
      }

      if(pthread_detach(threads_[i]) != 0) {
        delete [] threads_;
        throw std::exception();
      }

    }
}

template<typename T>
Threadpool<T>:: ~Threadpool(){
  delete [] threads_;
  stop_ = true;
}

template<typename T>
bool Threadpool<T> :: Append(T *request) {
  queuelock_.Lock();
  if(workqueue_.size() > maxrequests_) {
    queuelock_.UnLock();
    return false;
  }
  workqueue_.push_back(request);
  queuelock_.UnLock();
  queuestat_.Post();
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
  while(!stop_) {
    queuestat_.Wait();
    queuelock_.Lock();
    T * request = workqueue_.front();
    workqueue_.pop_front();
    queuelock_.UnLock();
    request->Process();
  }
}

#endif // THREADPOOL_H
