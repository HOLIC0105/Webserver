#if !defined(LOCKER_H)
#define LOCKER_H

#include <pthread.h>
#include <vector>
#include <sys/socket.h>
class locker{
  public:
    locker() {
      if(pthread_mutex_init(&mutex_, NULL) == -1) {
        throw;
      } 
    }

  private:
    pthread_mutex_t mutex_ ;

}



#endif // LOCKER_H
