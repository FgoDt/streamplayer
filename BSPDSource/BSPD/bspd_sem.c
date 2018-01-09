#include "bspd_sem.h"
#include "bspd_coder.h"
#include "bspd_mutex.h"

BSPDSem * bspd_create_sem(UINT32 inital_val) {
    BSPDSem *sem;

    sem = (BSPDSem*)malloc(sizeof(*sem));
    if (sem)
    {
        sem->id = CreateSemaphore(NULL, inital_val, 32 * 1024, NULL);
        sem->count = inital_val;
        if (!sem->id)
        {
            free(sem);
            sem = NULL;
        }
    }

    return sem;
}

void bspd_destroy_sem(BSPDSem *sem) {
    if (sem)
    {
        if (sem->id)
        {
            CloseHandle(sem->id);
            sem->id = 0;
        }
        free(sem);
    }
}

int bspd_sem_wait_timeout(BSPDSem* sem, UINT32 timeout) {
    int retval;
    DWORD dwMillseconds;
    if (!sem)
    {
        return BSPD_USE_NULL_ERROR;
    }

    if (timeout == MUTEX_MAXWAIT )
    {
        dwMillseconds = INFINITE;
    }
    else
    {
        dwMillseconds = (DWORD)timeout;
    }

    switch (WaitForSingleObject(sem->id,dwMillseconds))
    {
    case WAIT_OBJECT_0:
        InterlockedDecrement(&sem->count);
        retval = 0;
        break;
    case WAIT_TIMEOUT:
        retval = BSPD_MUTEX_TIMEOUT;
        break;
    default:
        retval = BSPD_ERRO_UNDEFINE;
        break;
    }

    return retval;
}

int bspd_sem_try_wait(BSPDSem *sem) {
    return bspd_sem_wait_timeout(sem, 0);
}

int bspd_sem_wait(BSPDSem *sem) {
    return bspd_sem_wait_timeout(sem, MUTEX_MAXWAIT);
}

int bspd_semval(BSPDSem *sem) {
    if (!sem)
    {
        return BSPD_USE_NULL_ERROR;
    }

    return (int)sem->count;
}

int bspd_sem_post(BSPDSem *sem) {
    if (!sem)
    {
        return BSPD_USE_NULL_ERROR;
    }

    InterlockedIncrement(&sem->count);
    if (ReleaseSemaphore(sem->id,1,NULL)==FALSE)
    {
        InterlockedDecrement(&sem->count);
        return BSPD_ERRO_UNDEFINE;
    }
    return BSPD_OP_OK;
}