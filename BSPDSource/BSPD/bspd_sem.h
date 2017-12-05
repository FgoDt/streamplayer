#ifndef __BSPD_SEM_H__
#define __BSPD_SEM_H__
#include <Windows.h>

typedef struct BSPDSemaphore {
    HANDLE id;
    long count;
}BSPDSem;

BSPDSem * bspd_create_sem(UINT32 inital_val);

void bspd_destroy_sem(BSPDSem *sem);

int bspd_sem_wait_timeout(BSPDSem* sem, UINT32 timeout);

int bspd_sem_try_wait(BSPDSem *sem);

int bspd_sem_wait(BSPDSem *sem);

int bspd_semval(BSPDSem *sem);

int bspd_sem_post(BSPDSem *sem);

#endif // !__BSPD_SEM_H__
