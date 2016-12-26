#ifndef VOLTE_DEVICE_H
#define VOLTE_DEVICE_H

#define DEVICE_TERMINAL        0
#define DEVICE_A_SBC           1
#define DEVICE_AGCF            2
#define DEVICE_IP_PBX          3
#define DEVICE_P_CSCF          4
#define DEVICE_I_CSCF          5
#define DEVICE_S_CSCF          8
#define DEVICE_AS              9
#define DEVICE_MGCF            10
#define DEVICE_BGCF            11
#define DEVICE_IBCF            12
#define DEVICE_MRFC            13
#define DEVICE_ENUM_SERVER     14
#define DEVICE_SLF             15
#define DEVICE_IP_SM_GW        16
#define DEVICE_ATCF            17
#define DEVICE_PCRF            18
#define DEVICE_S_GW            19
#define DEVICE_P_GW            20
#define DEVICE_HSS             21
#define DEVICE_MME             22
#define DEVICE_eMSC            23
#define DEVICE_UNKNOWN         255

typedef volte_device_t {
	uint8_t dev_type;
} volte_device;

#endif

