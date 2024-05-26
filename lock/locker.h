#ifndef LOCKER_H

#define LOCKER_H

#include <pthread.h>
#include <semaphore.h>
#include <exception>

// mutex互斥锁
class locker
{
public:
    locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
            throw std::exception();
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t* get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

// PV-sem信号量
class sem
{
public:
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
            throw std::exception();
    }
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
            throw std::exception();
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
private:
    sem_t m_sem;
};

// condVar条件变量
class cond
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, 0) != 0)
            throw std::exception();
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t* mutex)
    {
        return pthread_cond_wait(&m_cond, mutex);
    }
    bool timewait(pthread_mutex_t* mutex, struct timespec abstime)
    {
        return pthread_cond_timedwait(&m_cond, mutex, &abstime) == 0;
    }
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
private:
    pthread_cond_t m_cond;
};

// rwlock读写锁
class rwlock
{
public:
    rwlock()
    {
        if (pthread_rwlock_init(&m_rwlock, NULL) != 0)
            throw std::exception();
    }
    ~rwlock()
    {
        pthread_rwlock_destroy(&m_rwlock);
    }
    bool rdlock()
    {
        return pthread_rwlock_rdlock(&m_rwlock);
    }
    bool wrlock()
    {
        return pthread_rwlock_wrlock(&m_rwlock);
    }
    bool unlock()
    {
        return pthread_rwlock_unlock(&m_rwlock);
    }
    pthread_rwlock_t* get()
    {
        return &m_rwlock;
    }

private:
    pthread_rwlock_t m_rwlock;
};

// spinlock自旋锁
class spin
{
public:
    spin()
    {
        if (pthread_spin_init(&m_spin, 0) != 0)
            throw std::exception();
    }
    ~spin()
    {
        pthread_spin_destroy(&m_spin);
    }
    bool lock()
    {
        return pthread_spin_lock(&m_spin) == 0;
    }
    bool unlock()
    {
        return pthread_spin_unlock(&m_spin) == 0;
    }
    pthread_spinlock_t* get()
    {
        return &m_spin;
    }


private:
    pthread_spinlock_t m_spin;
};

#endif // LOCKER_H