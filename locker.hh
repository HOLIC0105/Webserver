#if !defined(LOCKER_H)
#define LOCKER_H

#include <pthread.h>
#include <exception>

class Locker{
  public:
    Locker() {
      if(pthread_mutex_init(&mutex_, NULL) != 0) {
        throw std::exception();
      }
    }

    ~Locker() {
      pthread_mutex_destroy(&mutex_);
    }

    bool Lock() {
      return pthread_mutex_lock(&mutex_) == 0;
    }

    bool UnLock() {
      return pthread_mutex_unlock(&mutex——) == 0;
    }

    pthread_mutex_t * Get() {
      return &mutex_;
    }


  private:
    pthread_mutex_t mutex_;

};

class Cond{
  public:
  Cond() {
    if(pthread_cond_init(&cond_, NULL) != 0) {
      throw std::exception();
    }
  }

  ~Cond() {
    pthread_cond_destroy(&cond_);
  }

  bool Wait(pthread_mutex_t * mutex) {
    return pthread_cond_wait(&cond_, mutex) == 0;
  }

  bool TimeWait(pthread_mutex_t * mutex, timespec *tim) {
    return pthread_cond_timedwait(&cond_, mutex, tim);
  }


  private:
    pthread_cond_t cond_;
}


#endif // LOCKER_H
