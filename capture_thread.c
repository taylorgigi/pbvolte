#include "global.h"
#include "config.h"
#include "pkt_pool.h"
#include "rte_atomic.h"
#include "pfring.h"
#include "capture_thread.h"
#include <pthread.h>
#include <string.h>
#include <pcap/pcap.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

list_t dispatch_queue;

// callback function, will be called when number of packets captured reached 'cnt' in pcap_loop()
void packet_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
	cap_statis *statis = (cap_statis*)user;
	statis->byte_cur += h->caplen;
	node_t *n = NULL;
	struct timespec req = {0, 10};
	while((n = pkt_pool_alloc(&pack_pool)) == NULL) {
		nanosleep(&req, NULL);
	}
	pkt_buffer *pkt = (pkt_buffer*)n->data;
	memcpy(pkt->pkt, bytes, h->caplen);
	pkt->len = h->caplen;
	pkt->ts = h->ts;
	// put packet to dispatch queue
	list_push(&dispatch_queue, n);
}

// packet reading thread on live mode using pfring
void *live_thread_pfring(void *arg) {
	// to-do: set cpu affinity
	pthread_detach(pthread_self());
	rte_atomic32_inc(&prog_ctl.thr_num);
	unsigned long idx = (unsigned long)arg;
	char *dev = glb_config.live.device[idx];
	// packets and byte statistics
	cap_statis statis = {0, 0, 0, 0, {0, 0}, {0, 0}};
	gettimeofday(&statis.tv_old, NULL);

        //pfring *pfhdl = pfring_open_dna(dev, 1, 0);
        pfring *pfhdl = pfring_open(dev, 1, SNAPSHOT_LEN, 0);
        if(pfhdl == NULL) {
		rte_atomic32_dec(&prog_ctl.thr_num);
		pthread_exit(NULL);
        }
        if(pfring_enable_ring(pfhdl) < 0) {
		printf("pfring_enable_ring error,%s,%d\n",__FILE__,__LINE__);
		goto CLEANUP_AND_EXIT;
		//pfring_close(pfhdl);
		//rte_atomic32_dec(&prog_ctl.thr_num);
		//pthread_exit(NULL);
        }
	char ringname[128] = {0};
	sprintf(ringname, "pbvolte_ring%lu", idx);
	if(pfring_set_application_name(pfhdl, ringname) < 0) {
		printf("pfring_set_application_name error,%s,%d\n",__FILE__,__LINE__);
		goto CLEANUP_AND_EXIT;
		//pfring_close(pfhdl);
		//rte_atomic32_dec(&prog_ctl.thr_num);
		//pthread_exit(NULL);
	}
	// capture incoming packet only
	if(pfring_set_direction(pfhdl, rx_only_direction) < 0) {
		printf("pfring_set_direction error,%s,%d\n",__FILE__,__LINE__);
		goto CLEANUP_AND_EXIT;
		//pfring_close(pfhdl);
		//rte_atomic32_dec(&prog_ctl.thr_num);
		//pthread_exit(NULL);
	}
	// read packet from pfring
	int retcode = 0, delay = 0;
	struct pfring_pkthdr hdr;
	pfring_stat stat = {0, 0};
	char buf[SNAPSHOT_LEN] = {0};
	char *bufp = buf;
	node_t *n = NULL;
	pkt_buffer *pktbuf = NULL;
	struct timespec req = {0, 10};
        while(rte_atomic32_read(&prog_ctl.run)) {
		retcode = pfring_recv(pfhdl, (u_char**)&bufp, SNAPSHOT_LEN, &hdr, WAIT);
		if(retcode == 1) { // success
			++statis.pkt_cur;
			// allocate memory for store packet and push packet to dispatch_queue ====>
			while((n = pkt_pool_alloc(&pack_pool)) == NULL) {
				nanosleep(&req, NULL);
			}
			pktbuf = (pkt_buffer*)n->data;
			pktbuf->len = hdr.caplen;
			pktbuf->ts = hdr.ts;
			memcpy(pktbuf->pkt, buf, hdr.caplen);
			list_push(&dispatch_queue, n);
			// <==== allocate memory for store packet and push packet to dispatch_queue
			// pkt speed and pfring recv drop statistics ====>
			gettimeofday(&statis.tv_cur, NULL);
			delay = statis.tv_cur.tv_sec - statis.tv_old.tv_sec;
			if(delay >= 60) {
				pfring_stats(pfhdl, &stat);
				printf("Live NIC %s, %s recv:%lu drop:%lu, %.2f pps\n",dev,ringname,stat.recv,stat.drop,(statis.pkt_cur-statis.pkt_old)/(float)delay);
				statis.pkt_old = statis.pkt_cur;
				statis.tv_old = statis.tv_cur;
			}
			// <==== pkt speed and pfring recv drop statistics
		}
		else if(retcode == -1) { // error
			printf("pfring_recv error,%s,%d\n",__FILE__,__LINE__);
			goto CLEANUP_AND_EXIT;
		}
	}
CLEANUP_AND_EXIT:
	// clean up and exit
        pfring_close(pfhdl);
	rte_atomic32_dec(&prog_ctl.thr_num);
	pthread_exit(NULL);
	return NULL;
}

// packet reading thread on live mode using libpcap
void *live_thread_pcap(void *arg) {
	// to-do: set cpu affinity
	pthread_detach(pthread_self());
	rte_atomic32_inc(&prog_ctl.thr_num);
	unsigned long idx = (unsigned long)arg;
	char *dev = glb_config.live.device[idx];
	// packets and byte statistics
	cap_statis statis = {0, 0, 0, 0, {0, 0}, {0, 0}};
	gettimeofday(&statis.tv_old, NULL);

	pcap_t *hdl = NULL;
	char errbuf[PCAP_ERRBUF_SIZE] = {0};
	hdl = pcap_open_live(dev, SNAPSHOT_LEN, 1, 0, errbuf);
	if(hdl == NULL) {
		printf("pcap_open_live error,%s,%s,%d\n",errbuf,__FILE__,__LINE__);
		rte_atomic32_dec(&prog_ctl.thr_num);
		pthread_exit(NULL);
	}
	if(errbuf[0])
		printf("Warning: interface %s %s,%s,%d\n", dev, errbuf, __FILE__, __LINE__);

	uint16_t delay = 0;
	while(rte_atomic32_read(&prog_ctl.run)) {
		if(pcap_loop(hdl, 10000, packet_callback, (u_char*)&statis) == -1) {
			printf("pcap_loop error, %s, %s, %d\n", pcap_geterr(hdl), __FILE__, __LINE__);
			rte_atomic32_dec(&prog_ctl.thr_num);
			pcap_close(hdl);
			pthread_exit(NULL);
		}
		statis.pkt_cur += 10000;
		gettimeofday(&statis.tv_cur, NULL);
		delay = statis.tv_cur.tv_sec - statis.tv_old.tv_sec;
		if(delay >= 60) {
			printf("Live capture: device %s %.2fpkt/s %.2fbytes/s\n", dev, (statis.pkt_cur - statis.pkt_old)/(float)delay, (statis.byte_cur - statis.byte_old)/(float)delay);
			statis.pkt_old = statis.pkt_cur;
			statis.byte_old = statis.byte_cur;
			statis.tv_old = statis.tv_cur;
		}
	}

	pcap_close(hdl);
	rte_atomic32_dec(&prog_ctl.thr_num);
	pthread_exit(NULL);
	return NULL;
}

// 
int get_pcap_files(list_t *filelist, const char *dir) {
	struct dirent *ent = NULL;
	DIR *dirh = opendir(glb_config.offline.dir);
	if(dirh == NULL) {
		printf("opendir error,%s,%d\n",__FILE__,__LINE__);
		return -1;
	}
	node_t *n = NULL;
	while((ent = readdir(dirh)) != NULL) {
		int nlen = ent->d_reclen;
		if(nlen < 4)
			continue;
		if(strcasestr(ent->d_name,".cap") || strcasestr(ent->d_name,".pcap")) {
			n = calloc(1,sizeof(node_t));
			if(n) {
				n->data = strdup(ent->d_name);
				list_push(filelist, n);
			}
		}
	}
	closedir(dirh);
	return 0;
}

int read_pcap_file(const char *filename) {
	char errbuf[PCAP_ERRBUF_SIZE] = {0};
	char name[512] = {0};
	sprintf(name, "%s/%s", glb_config.offline.dir, filename);
	pcap_t *file = pcap_open_offline(name, errbuf);
	if(file == NULL) {
		printf("pcap_open_offline error,%s,%s,%d\n",errbuf,__FILE__,__LINE__);
		return -1;
	}
	unsigned long sum = 0;
	int cnt = 0;
	cap_statis statis = {0, 0, 0, 0, {0, 0}, {0, 0}};
	while(1) {
		cnt = pcap_dispatch(file, 1, packet_callback, (u_char*)&statis);
		if(cnt < 0) {
			printf("pcap_dispatch error,%s,%d\n",__FILE__,__LINE__);
			pcap_close(file);
			return -1;
		}
		else if(cnt == 0)
			break;
		else
			sum += cnt;
	}
	pcap_close(file);
	//printf("%lu packets, %lu bytes treated in %s\n", sum, statis.byte_cur, name);
	return 0;
}

// packet reading pthread on offline mode
void *offline_thread(void *arg) {
	// to-do: set cpu affinity
	pthread_detach(pthread_self());

	rte_atomic32_inc(&prog_ctl.thr_num);

	node_t *n = NULL;
	list_t file_list;
	list_create(&file_list);

	do {
		if(get_pcap_files(&file_list, glb_config.offline.dir) < 0) {
			printf("get_pcap_files error, %s,%d\n",__FILE__,__LINE__);
			break;
		}
		n = file_list.head;
		while(n) {
			if(read_pcap_file(n->data) < 0) {
				printf("read_pcap_file error,%s,%d\n",__FILE__,__LINE__);
			}
			// switch to the next file
			n = n->next;
		}
		// === free list
		while((n = list_pop(&file_list)) != NULL) {
			free(n->data);
			free(n);
		}
		// free list ===
	} while(rte_atomic32_read(&prog_ctl.run) && glb_config.offline.repeat);

	pthread_spin_destroy(&file_list.lock);
	rte_atomic32_dec(&prog_ctl.thr_num);
	pthread_exit(NULL);
	return NULL;
}

