#pragma once

/*******************************************************
/*                      UDP server
/*
/* Note that the server will dispatch messages to all clients
/*      that's mean there is no head in this connection
/*   the message header will determine which is the reciever
/*   for game states this depend in the game implementation
/*
/*
*********************************************************/
#include "extra/types.h"

// must be called in deffrent thread
err start_server(port_t p_port);
err server_get_data(buffer_t p_buff);
void server_dispatch_data(buffer_t p_buff);
void close_server();