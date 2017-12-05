#ifndef __BSPD_mutex_H__
#define __BSPD_mutex_H__

#ifdef _WIN64
#include <Windows.h>
#endif // _WIN32||WIN64

#define BSPD_MUTEX_TIMEOUT -2
#define MUTEX_MAXWAIT (~(UINT32)0)

typedef struct BSPDMutex {

#ifdef _WIN64
    CRITICAL_SECTION cs;
#endif // _WIN32||WIN64

}BSPDMutex;



BSPDMutex* bspd_create_mutex();

void bspd_destroy_mutex(BSPDMutex *mutex);

int bspd_try_lock_mutex(BSPDMutex *mutex);

int bspd_lock_mutex(BSPDMutex *mutex);

int bspd_unlock_mutex(BSPDMutex *mutex);

#endif // !__BSPD_mutex_H__
