#pragma once
#include "extra/types.h"

// curently support just 2 ends


extern mode_e current_mode;

struct NET_HANDLER {
    void (*create_server)(const port_t p_port);
    void (*connect_to)(const string_t p_addr, const port_t p_port);
    void (*connect_sig)(const signal_e p_sig, const callable_t p_func);
    void (*send_data)(buffer_t p_buff); // the buffer should not be const as net should edit the header
    void (*close)();
};

extern const struct NET_HANDLER Net_Handler;