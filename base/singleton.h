#pragma once
#include <pthread.h>
namespace grit {

template <typename T>
class Singleton
{
  public:
    virtual ~Singleton() {}

    static T *getInstance()
    {
        pthread_once(&m_InstanceFlag, &Singleton::initInstance);
        return m_pInstance;
    }
    static void destroyInstance() { delete m_pInstance; }

  protected:
    // can't new
    Singleton() {}

  private:
    static pthread_once_t m_InstanceFlag; // to protect m_pInstance
    static T *m_pInstance;                // singleton

  private:
    // can't copy
    Singleton(const Singleton &s);
    Singleton &operator=(const Singleton &s);

    // init m_pInstance
    static void initInstance() { m_pInstance = new T(); }
};

template <typename T>
pthread_once_t Singleton<T>::m_InstanceFlag = PTHREAD_ONCE_INIT;

template <typename T>
T *Singleton<T>::m_pInstance = NULL;

} // namespace grit