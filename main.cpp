#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "IPortingPlayer.h"
#include "FFMPEGPlayer.h"
#include "ISurface.h"
#include "SDL2Surface.h"
#include "type.h"
#include "lxlog.h"
#include "lxMsgQue.h"
#include "lxOs.h"


void notify(int msg, int ext1) {
    LOG_ERROR_PRINT("msgid:%d, ext1:%d\n", msg, ext1);

    return;
}

int main(int argc, char * argv[]) {

    surfaceRect rect = {0,0,720,576};
    ISurface * surface = new SDL2Surface();
    surface->createSurface(rect);
    IPortingPlayer * player = new FFMPEGPlayer();
    player->setSurface(surface);
    player->setListener(notify);

    player->setDataSource("/home/u12/av/test.avi");
    player->prepare();
    player->start();

    while(1) {
        usleep(30 * 1000);
    }

    return 0;
}