#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

void daemonize(void);
void parse_args(int argc, char** argv, int* do_daemon, char* port, void (*usage)(void));

#endif /* UTILS_H_INCLUDED */
