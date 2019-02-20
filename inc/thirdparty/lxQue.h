#ifndef __LX_QUE_H__
#define __LX_QUE_H__

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lxque_obj {

    status_e  (*push_back)(struct lxque_obj *pThis, const void * data, size_t len);
    size_t    (*pop_front)(struct lxque_obj *pThis, void * outbuffer, size_t bufferlen);
    BOOLTYPE  (*isEmpty)(struct lxque_obj *pThis);

}lxque_obj;

lxque_obj * lxque_obj_new(void);
void lxque_obj_delete(lxque_obj * pThis);

#ifdef __cplusplus
}
#endif

#endif

