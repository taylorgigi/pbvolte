#include "config.h"
#include "iniparser/iniparser.h"
#include <string.h>

global_config glb_config;

int load_config(const char *cfgfile) {
	int i = 0;
	char *s = NULL;
	dictionary *dic = NULL;
	memset(&glb_config, 0, sizeof(global_config));
	dic = iniparser_load(cfgfile);
	if(dic == NULL)
		return -1;
	// get capture mode
	s = iniparser_getstring(dic, "Mode:mode", NULL);
	if(s) {
		if(strcasecmp(s, "live") == 0)
			glb_config.mode = 1;
		else if(strcasecmp(s, "offline") == 0)
			glb_config.mode = 2;
		else
			glb_config.mode = 1;
	}
	// get number of transaction threads
	glb_config.nb_thr = iniparser_getint(dic, "Thread:count", 1);
	// live mode, get number of devices
	glb_config.live.count = iniparser_getint(dic, "Live:count", 0);
	if(glb_config.live.count > 0)
		glb_config.live.device = calloc(glb_config.live.count, sizeof(char*));
	char buf[16] = {0};
	// live mode, get name of devices
	for(i=1; i<=glb_config.live.count; ++i) {
		memset(buf, 0, 16);
		sprintf(buf, "Live:dev%d", i);
		s = iniparser_getstring(dic, buf, NULL);
		if(s)
			glb_config.live.device[i-1] = strdup(s);
	}
	s = iniparser_getstring(dic, "Offline:dir", NULL);
	if(s)
		glb_config.offline.dir = strdup(s);
	glb_config.offline.repeat = iniparser_getint(dic, "Offline:repeat", 0);
	s = iniparser_getstring(dic, "Xdr:dir", NULL);
	if(s)
		glb_config.xdr.dir = strdup(s);
	s = iniparser_getstring(dic, "Xdr:surffix", NULL);
	if(s)
		glb_config.xdr.surffix = strdup(s);
	glb_config.pool.pkt = iniparser_getint(dic, "Pool:pkt", 0);
	glb_config.pool.ipfrag = iniparser_getint(dic, "Pool:ipfrag", 0);
	glb_config.pool.sctpfrag = iniparser_getint(dic, "Pool:sctpfrag", 0);
	glb_config.pool.tcpfrag = iniparser_getint(dic, "Pool:tcpfrag", 0);
	glb_config.pool.diameter = iniparser_getint(dic, "Pool:diameter", 0);
	glb_config.pool.gtpv2 = iniparser_getint(dic, "Pool:gtpv2", 0);
	glb_config.pool.sip = iniparser_getint(dic, "Pool:sip", 0);
	glb_config.pool.rtp = iniparser_getint(dic, "Pool:rtp", 0);

	glb_config.hash.ipfrag = iniparser_getint(dic, "Hash:ipfrag", 0);
	glb_config.hash.sctpfrag = iniparser_getint(dic, "Hash:sctpfrag", 0);
	glb_config.hash.tcpfrag = iniparser_getint(dic, "Hash:tcpfrag", 0);
	glb_config.hash.diameter = iniparser_getint(dic, "Hash:diameter", 0);
	glb_config.hash.gtpv2 = iniparser_getint(dic, "Hash:gtpv2", 0);
	glb_config.hash.sip = iniparser_getint(dic, "Hash:sip", 0);
	glb_config.hash.rtp = iniparser_getint(dic, "Hash:rtp", 0);

	glb_config.timeout.ipfrag = iniparser_getint(dic, "Timeout:ipfrag", 0);
	glb_config.timeout.sctpfrag = iniparser_getint(dic, "Timeout:sctpfrag", 0);
	glb_config.timeout.tcpfrag = iniparser_getint(dic, "Timeout:tcpfrag", 0);
	glb_config.timeout.diameter = iniparser_getint(dic, "Timeout:diameter", 0);
	glb_config.timeout.gtpv2 = iniparser_getint(dic, "Timeout:gtpv2", 0);
	glb_config.timeout.sip = iniparser_getint(dic, "Timeout:sip", 0);
	glb_config.timeout.rtp = iniparser_getint(dic, "Timeout:rtp", 0);
	iniparser_freedict(dic);
	return 0;
}

