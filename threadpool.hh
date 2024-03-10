#if !defined(THREADPOOL_H)
#define THREADPOOL_H

#include <list>
#include "locker.hh"

//threadpool T is task class
template<typename T> 
class threadpool{
  public:
  private:
    constexpr int threadnum_;
    pthread_t * threads_; 
    int maxrequests_;
    locker queuelock_;
    sem queuestat_;
    bool stop_;
    std::list<T> workqueue;
};


#endif // THREADPOOL_H
