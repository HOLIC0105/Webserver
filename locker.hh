#ifndef LOCKER_H
#define LOCKER_H


#include <pthread.h>
#include <exception>
#include <semaphore.h>

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
      return pthread_mutex_unlock(&mutex_) == 0;
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

    bool TimedWait(pthread_mutex_t * mutex, timespec *tim) {
      return pthread_cond_timedwait(&cond_, mutex, tim) == 0;
    }

    bool Signal() {
      return pthread_cond_signal(&cond_) == 0;
    }

    bool Broadcast() {
      return pthread_cond_broadcast(&cond_) == 0;
    }

  private:
    pthread_cond_t cond_;
};

class Sem{
  public:
    Sem() {
      if(sem_init(&sem_, 0, 0) != 0) {
        throw std::exception();
      }
    }

    Sem(int num) {
      if(sem_init(&sem_, 0, num) != 0) {
        throw std::exception();
      }
    }

    ~Sem() {
      sem_destroy(&sem_);
    }

    bool Wait() {
      return sem_wait(&sem_) == 0;
    }

    bool Post() {
      return sem_post(&sem_) == 0;
    }

  private:
    sem_t sem_; //semaphore.h
};


#endif // LOCKER_H
