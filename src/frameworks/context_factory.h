/*
    AppContext creation and initialisation.
 */

#ifndef CONTEXT_FACTORY_H
#define CONTEXT_FACTORY_H

#include "app_context.h"

AppContext *context_create(void);
int  context_init_ipc(AppContext *ctx);
int  context_open_media(AppContext *ctx, const char *path);
void context_close_sockets(AppContext *ctx);
void context_teardown(AppContext *ctx);

#endif
