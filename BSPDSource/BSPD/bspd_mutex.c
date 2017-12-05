#include "bspd_mutex.h"
#include "bspd_coder.h"

BSPDMutex* bspd_create_mutex() {
    BSPDMutex *mutex;
    mutex = (BSPDMutex*)malloc(sizeof(BSPDMutex));

    if (mutex)
    {
        InitializeCriticalSectionAndSpinCount(&mutex->cs, 2000);
    }
    else
    {
        return NULL;
    }

    return mutex;

}

void bspd_destroy_mutex(BSPDMutex *mutex) {
    if (mutex)
    {
        DeleteCriticalSection(&mutex->cs);
        free(mutex);
    }
}

int bspd_try_lock_mutex(BSPDMutex *mutex) {
    int ret = BSPD_OP_OK;
    if (mutex == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (TryEnterCriticalSection(&mutex->cs) == 0)
    {
        ret = BSPD_MUTEX_TIMEOUT;
    }
    return ret;
}

int bspd_lock_mutex(BSPDMutex *mutex) {
    if (mutex == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }

    EnterCriticalSection(&mutex->cs);

    return BSPD_OP_OK;
}

int bspd_unlock_mutex(BSPDMutex *mutex) {

    if (mutex == NULL)
    {
        return BSPD_USE_NULL_ERROR;
    }

    LeaveCriticalSection(&mutex->cs);
    return BSPD_OP_OK;
}