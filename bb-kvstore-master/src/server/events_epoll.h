#ifndef EVENTS_EPOLL_H_INCLUDED
#define EVENTS_EPOLL_H_INCLUDED

#include <stdlib.h>
#include <stddef.h>

#include <sys/epoll.h>

static int
_events_modify_context(int epollfd, int operation, int fd, uint32_t events, void* data)
{
	struct epoll_event event;
	event.events = events;
	event.data.ptr = data;

	return epoll_ctl(epollfd, operation, fd, &event);
}

static int
events_create_context()
{
	int epoll_ctx = epoll_create(1);
	return epoll_ctx;
}

static int
events_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	return epoll_wait(epfd, events, maxevents, timeout);
}

static void
events_register_socket(int epollfd, int sockefd)
{
	_events_modify_context(epollfd, EPOLL_CTL_ADD, sockefd, EPOLLET, 0);
}

static void
events_set_read_state(int epollfd, int sockefd, void* data)
{
	_events_modify_context(epollfd, EPOLL_CTL_MOD, sockefd, EPOLLIN | EPOLLET, data);
}

static void
events_set_write_state(int epollfd, int sockefd, void* data)
{
	_events_modify_context(epollfd, EPOLL_CTL_MOD, sockefd, EPOLLOUT | EPOLLET, data);
}

static struct epoll_event*
events_create_events_array(size_t max_events)
{
	return calloc(max_events, sizeof(struct epoll_event));
}

static void
events_destroy_events_array(void* epoll_events)
{
	free(epoll_events);
}

static void*
events_get_user_data(void* epoll_events, size_t event_index)
{
	struct epoll_event* events = (struct epoll_event *) epoll_events;

	return events[event_index].data.ptr;
}

static int
events_is_error_event(void* epoll_events, size_t event_index)
{
	struct epoll_event* events = (struct epoll_event *) epoll_events;

	return (
		events[event_index].events & EPOLLHUP ||
		events[event_index].events & EPOLLERR
	);
}

#endif /* EVENTS_EPOLL_H_INCLUDED */
