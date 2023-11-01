#include "net.h"
#include <pthread.h>

// include based in os
#ifdef UNIX
    #include "unix/server.h"
    #include "unix/client.h"
#elif WIN // if I want to port this to windows
    #error "Not PORTED TO WIN YET"
    // #include "win/server.h"
    // #include "win/client.h"
#else
    #error "No TARGET PLATFORM"
#endif

static callable_t sig_callback[
    SIGNALS
] = {0};

mode_e current_mode = NONE;
static pthread_t net_thread;
extern char alias[8];

static _server_thread() {
    char buffer[MAX_BUFFER_SIZE];
    while (1)
    {
        err _err = server_get_data(buffer);
        if(_err) continue;
        if(sig_callback[RECV_DATA]) {
        sig_callback[RECV_DATA](buffer);}
        usleep(DELTA);
    }
    pthread_exit(null);
}

static _client_thread() {
    byte_t buffer[MAX_BUFFER_SIZE];
    MSG_HEADER* msg = buffer;
    while (1)
    {
        err _err = client_get_data(buffer);
        if (_err) continue;
        if (msg->op == 2) continue;
        if(sig_callback[RECV_DATA]) sig_callback[RECV_DATA](buffer);
        usleep(DELTA);
    }
    pthread_exit(null);
}

static void _create_server(const port_t p_port) {
    if(start_server(p_port)) {
        return;
    }
    pthread_create(&net_thread, null, _server_thread, null);
    current_mode = SERVER;
    if (sig_callback[CONNECTED]) sig_callback[CONNECTED]();
}

static void _connect_to(const string_t p_addr, const port_t p_port) {
    if(start_client(p_addr, p_port)) {
        return;
    }
    pthread_create(&net_thread, null, _client_thread, null);
    current_mode = CLIENT;
    if (sig_callback[CONNECTED]) sig_callback[CONNECTED]();
}

static void _connect_sig(const signal_e p_sig, const callable_t p_func) {
    if (sig_callback[p_sig])
    {
        ERR("Can't connect signal, preconnected.")
        return;
    }
    sig_callback[p_sig] = p_func;
}

static void _send_data(buffer_t p_buff) {
    if (current_mode == SERVER) {
        server_dispatch_data(p_buff);
    } else if (current_mode == CLIENT) {
        err _err = client_dispatch_data(p_buff);
        if (_err == ERR_CANNOT_SEND_DATA) {
            if (sig_callback[DISCONNECTED]) sig_callback[DISCONNECTED]();
            close_client();
        }
    }
    
}

void _close() {
    if (current_mode == SERVER) {
        close_server();
    } else if (current_mode == CLIENT) {
        close_client();
    }
}

const struct NET_HANDLER Net_Handler = {
    .create_server = _create_server,
    .connect_to = _connect_to,
    .connect_sig = _connect_sig,
    .send_data = _send_data,
    .close = _close,
};