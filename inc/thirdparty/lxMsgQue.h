#ifndef __LX_MSGQUE_H__
#define __LX_MSGQUE_H__

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msgque_obj {

    status_e  (*push_back)(struct msgque_obj *pThis, const void * data, size_t datalen);
    VOIDPTR   (*pop_front)(struct msgque_obj *pThis);
    VOIDPTR   (*pop_front_timeout)(struct msgque_obj *pThis, unsigned char timeoutS);
    void      (*release_buffer)(struct msgque_obj *pThis, void * data);
}msgque_obj;

msgque_obj * msgque_obj_new(void);
void msgque_obj_delete(msgque_obj * pThis);

#ifdef __cplusplus
}
#endif

#endif