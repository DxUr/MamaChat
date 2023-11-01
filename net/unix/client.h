#pragma once

/*******************************************************
/*                      UDP client
*********************************************************/
#include "extra/types.h"

// must be called in deffrent thread
err start_client(string_t p_addr, port_t p_port);
err client_get_data(buffer_t p_buff);
err client_dispatch_data(buffer_t p_buff);
void close_client();