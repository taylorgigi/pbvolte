#include "config.h"
#include "rte_atomic.h"
#include "pkt_pool.h"
#include "global.h"
#include "capture_thread.h"
#include "dispatch_thread.h"
#include "transaction_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

int set_nonblock(int fd) {
	int oldflag;
	oldflag = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, oldflag | O_NONBLOCK);
	return oldflag;
}

void epoll_addfd(int epollfd, int fd, int ev){
	struct epoll_event event;
	event.events = ev;
	event.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	set_nonblock(fd);
}

int epoll_init() {
	efd = epoll_create(1);
	if(efd < 0) {
		printf("Failed to create epoll,%s,%d\n", __FILE__, __LINE__);
		return -1;
	}
	epoll_addfd(efd,sigfd,EPOLLIN);
	return 0;
}

int signal_init() {
	// signal block
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGKILL);
	sigaddset(&mask, SIGTERM);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	// create signal file descriptor and add to epoll control
	if((sigfd = signalfd(-1, &mask, 0)) < 0) {
		printf("Failed to create signalfd, %s, %d\n", __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

// parse command line arguments and load config
int parse_prog_parameters(int argc, char *argv[]) {
	char progname[256] = {0};
	char cfgfile[256] = {0};

	if(argc < 3) {
		strcpy(progname, argv[0]);
		printf("usage: %s -c <configfile>\n", basename(progname));
		return EXIT_FAILURE;
	}
	int optchar = 0;
	while((optchar = getopt(argc, argv, "c:")) != -1) {
		switch(optchar) {
			case 'c':
				strcpy(cfgfile, optarg);
				break;
			default:
				break;
		}
	}
	if(!cfgfile[0])
		return EXIT_FAILURE;
	load_config(cfgfile);
	return EXIT_SUCCESS;
}

void signal_handle(int fd) {
	struct signalfd_siginfo si;
	struct timespec req = {0, 10000};
	int sl = sizeof si;
	memset(&si, 0, sl);
	if(read(fd, &si, sl) < 0) {
		printf("Failed to read sigfd, %s, %s, %d\n", __FUNCTION__, __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	//printf("Signal %d happens, will terminate system\n", si.ssi_signo);
	if(si.ssi_signo == SIGINT || si.ssi_signo == SIGKILL || si.ssi_signo == SIGTERM) {
		rte_atomic32_set(&prog_ctl.run, 0);
		// wait until all threads exit, clean up system resource.
		while(rte_atomic32_read(&prog_ctl.thr_num) > 0) {
			nanosleep(&req, NULL);
		}
	}
}

void event_loop() {
	int i;
	int num = 0;
	int fd = -1;
	uint32_t event = 0;
	while(rte_atomic32_read(&prog_ctl.run)) {
		num = epoll_wait(efd, &events, 1, -1);
		if(num < 0) {
			rte_atomic32_set(&prog_ctl.run, 0);
			exit(EXIT_FAILURE);
		}
		for(i=0; i<num; ++i) {
			fd = events.data.fd;
			event = events.events;
			// new client comes, accept it and add to epoll event loop
			if(fd==sigfd && (event & EPOLLIN)) {
				signal_handle(fd);
			}
		}
	}
}

// launch capture thread
void create_capture_thread() {
	pthread_t tid = 0;
	int i;
	switch(glb_config.mode) {
		case MODE_LIVE:
			for(i=0; i<glb_config.live.count; ++i)
				pthread_create(&tid,NULL,live_thread_pfring,(void*)(unsigned long)i);
			break;
		case MODE_OFFLINE:
			pthread_create(&tid,NULL,offline_thread,NULL);
			break;
	}
}

// launch threads
void launch_thread() {
	pthread_t tid = 0;
	// create thread, read NIC or pcap file
	create_capture_thread();
	// create packet dispatch thread
	pthread_create(&tid, NULL, dispatch_thread, NULL);
	// create transaction thread
	int i;
	for(i=0; i<glb_config.nb_thr; ++i) {
		pthread_create(&tid, NULL, transaction_thread, (void*)(unsigned long)i);
	}
}

// create transaction queue
int transac_queue_create() {
	int i;
	transac_queue = calloc(glb_config.nb_thr, sizeof(list_t));
	if(transac_queue == NULL)
		return -1;
	for(i=0; i<glb_config.nb_thr; ++i) {
		list_create(&transac_queue[i]);
	}
	return 0;
}

// resource initialization including:
// - packet pool
// - program control variable
// - signal
// - epoll
// - dispatch queue
int initialize() {
	// init prog ctl
	rte_atomic32_set(&prog_ctl.run, 1);
	rte_atomic32_init(&prog_ctl.thr_num);
	// signal init
	if(signal_init() < 0)
		return EXIT_FAILURE;
	// epoll create
	if(epoll_init() < 0)
		return EXIT_FAILURE;
	// create packet pool
	if(pkt_pool_create(&pack_pool, glb_config.pool.pkt) < 0)
		return EXIT_FAILURE;
	// create dispatch queue
	list_create(&dispatch_queue);
	// create transaction queue
	if(transac_queue_create() < 0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	// check program prarmeters and load system configuration
	if(parse_prog_parameters(argc, argv) != EXIT_SUCCESS) {
		printf("parse command line params error\n");
		return EXIT_FAILURE;
	}
	// initialize resource
	if(initialize() < 0) {
		printf("initialize error\n");
		return EXIT_FAILURE;
	}
	// create thread
	launch_thread();
	// handle signals
	event_loop();

	return EXIT_SUCCESS;
}

