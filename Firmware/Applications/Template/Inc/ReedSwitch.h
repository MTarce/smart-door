
#pragma once 

typedef enum {
    kOpen,kClose
}ReadState;

void ReedSwitchInit(void);

ReadState ReedSwitchGetState(void);

