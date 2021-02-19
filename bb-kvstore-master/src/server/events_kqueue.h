#ifndef EVENTS_KQUEUE_H_INCLUDED
#define EVENTS_KQUEUE_H_INCLUDED

#include <stdlib.h>
#include <stddef.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

static int
_events_modify_context(int kqfd, u_short operation, int fd, short events, void* data)
{
	struct kevent evSet;
	EV_SET(&evSet, fd, events, operation, 0, 0, data);
	return kevent(kqfd, &evSet, 1, NULL, 0, NULL);
}

static int
events_create_context()
{
	int kq = kqueue();
	return kq;
}

static int
events_wait(int kqfd, struct kevent* events, int maxevents, int timeout)
{
	return kevent(kqfd, NULL, 0, events, maxevents, NULL);
}

static void
events_register_socket(int kqfd, int sockefd)
{
	_events_modify_context(kqfd, EV_ADD | EV_CLEAR, sockefd, EVFILT_READ, 0);
}

static void
events_set_read_state(int kqfd, int sockefd, void* data)
{
	_events_modify_context(kqfd, EV_ADD | EV_CLEAR, sockefd, EVFILT_READ, data);
}

static void
events_set_write_state(int kqfd, int sockefd, void* data)
{
	_events_modify_context(kqfd, EV_ADD | EV_CLEAR, sockefd, EVFILT_WRITE, data);
}

static struct kevent*
events_create_events_array(size_t max_events)
{
	return calloc(max_events, sizeof(struct kevent));
}

static void
events_destroy_events_array(void* kq_events)
{
	free(kq_events);
}

static void*
events_get_user_data(void* kq_events, size_t event_index)
{
	struct kevent* events = (struct kevent *) kq_events;

	return events[event_index].udata;
}

static int
events_is_error_event(void* kq_events, size_t event_index)
{
	struct kevent* events = (struct kevent *) kq_events;

	return (
		events[event_index].flags & EV_EOF ||
		events[event_index].flags & EV_ERROR
	);
}

#endif /* EVENTS_KQUEUE_H_INCLUDED */
