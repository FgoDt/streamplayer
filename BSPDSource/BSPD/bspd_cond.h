#ifndef __BSPD_COND_H__
#define __BSPD_COND_H__
#include "bspd_mutex.h"
#include "bspd_sem.h"

typedef struct BSPDCond
{
    BSPDMutex *lock;
    int waiting;
    int signals;
    BSPDSem *wait_sem;
    BSPDSem *wait_done;
}BSPDCond;

BSPDCond* bspd_create_cond(void);

void bspd_destroy_cond(BSPDCond *cond);

int bspd_cond_signal(BSPDCond *cond);

int bspd_cond_broadcast(BSPDCond *cond);

int bspd_cond_wait_timeout(BSPDCond *cond, BSPDMutex *mutex, UINT32 ms);

int bspd_cond_wait(BSPDCond *cond, BSPDMutex *mutex);

#endif // !__BSPD_COND_H__
