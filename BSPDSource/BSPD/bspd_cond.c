#include "bspd_cond.h"
#include "bspd_coder.h"

BSPDCond* bspd_create_cond(void) {
    BSPDCond *cond = NULL;
    cond = (BSPDCond*)malloc(sizeof(BSPDCond));
    if (cond)
    {
        cond->lock = bspd_create_mutex();
        cond->wait_sem = bspd_create_sem(0);
        cond->wait_done = bspd_create_sem(0);
        cond->waiting = cond->signals = 0;
        if (!cond->lock || !cond->wait_sem || !cond->wait_done)
        {
            bspd_destroy_cond(cond);
            cond = NULL;
        }
    }

    return cond;
}

void bspd_destroy_cond(BSPDCond *cond) {
    if (cond)
    {
        if (cond->wait_sem)
        {
            bspd_destroy_sem(cond->wait_sem);
        }
        if (cond->wait_done)
        {
            bspd_destroy_sem(cond->wait_done);
        }
        if (cond->lock)
        {
            bspd_destroy_mutex(cond->lock);
        }

        free(cond);
    }
}

int bspd_cond_signal(BSPDCond *cond) {
    if (!cond)
    {
        return BSPD_USE_NULL_ERROR;
    }
    
    bspd_lock_mutex(cond->lock);
    if (cond->waiting>cond->signals)
    {
        cond->signals++;
        bspd_sem_post(cond->wait_sem);
        bspd_unlock_mutex(cond->lock);
        bspd_sem_wait(cond->wait_done);
    }
    else
    {
        bspd_unlock_mutex(cond->lock);
    }

    return BSPD_OP_OK;
}

int bspd_cond_broadcast(BSPDCond *cond) {
    if (!cond)
    {
        return BSPD_USE_NULL_ERROR;
    }

    bspd_lock_mutex(cond->lock);
    if (cond->waiting>cond->signals)
    {
        int i, num_waiting;

        num_waiting = (cond->waiting - cond->signals);
        cond->signals = cond->waiting;
        for ( i = 0; i < num_waiting; i++)
        {
            bspd_sem_post(cond->wait_sem);
        }

        bspd_unlock_mutex(cond->lock);
        for ( i = 0; i < num_waiting; i++)
        {
            bspd_sem_wait(cond->wait_done);
        }
    }
    else
    {
        bspd_unlock_mutex(cond->lock);
    }
    return BSPD_OP_OK;
}


int bspd_cond_wait_timeout(BSPDCond *cond, BSPDMutex *mutex, UINT32 ms) {
    int retval;
    if (!cond)
    {
        return BSPD_USE_NULL_ERROR;
    }

    bspd_lock_mutex(cond->lock);
    ++cond->waiting;
    bspd_unlock_mutex(cond->lock);

    bspd_unlock_mutex(mutex);

    if (ms == MUTEX_MAXWAIT)
    {
        retval = bspd_sem_wait(cond->wait_sem);
    }
    else
    {
        retval = bspd_sem_wait_timeout(cond->wait_sem, ms);
    }

    bspd_lock_mutex(cond->lock);
    if (cond->signals >0)
    {
        if (retval>0)
        {
            bspd_sem_wait(cond->wait_sem);
        }
        bspd_sem_post(cond->wait_sem);
        --cond->signals;
    }
    --cond->waiting;

    bspd_unlock_mutex(cond->lock);
    return retval;

}

int bspd_cond_wait(BSPDCond *cond,BSPDMutex *mutex) {
   return bspd_cond_wait_timeout(cond,mutex, MUTEX_MAXWAIT);
}

