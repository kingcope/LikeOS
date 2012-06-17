#include "etherboot.h"
#include "timer.h"
#include "dev.h"
#include "pci.h"
#include "sys/types.h"
#include "stdio.h"

char localip[] = "192.168.2.23";
unsigned char eth_addr[6]={0x00,0x00,0xe2,0x81,0xb8,0x97};

#define ARP_CLIENT	0
#define ARP_SERVER	1
#define ARP_GATEWAY	2
#define MAX_ARP		ARP_GATEWAY+1
#define ETH_ALEN		6	/* Size of Ethernet address */
#define ETH_HLEN		14	/* Size of ethernet header */
#define	ETH_ZLEN		60	/* Minimum packet */
#define	ETH_FRAME_LEN		1514	/* Maximum packet */
#define ETH_DATA_ALIGN		2	/* Amount needed to align the data after an ethernet header */
#define	ETH_MAX_MTU		(ETH_FRAME_LEN-ETH_HLEN)
#define __aligned __attribute__((aligned(16))) 

typedef enum {
 	DISABLE = 0,
 	ENABLE,
 	FORCE
} irq_action_t;

struct in_addr{
	int i;
};

struct arptable_t {
 	uint8_t node[6];
};

struct rom_info {
      	unsigned short	rom_segment;
      	unsigned short	rom_length;
     };

struct rom_info		rom;
struct arptable_t	arptable[MAX_ARP];
static char	packet[ETH_FRAME_LEN + ETH_DATA_ALIGN] __aligned;
int network_ready=0;

struct nic
{
 	struct dev	dev;  /* This must come first */
 	int		(*poll)P((struct nic *, int retrieve));
 	void		(*transmit)P((struct nic *, const char *d,
 				unsigned int t, unsigned int s, const char *p));
	void		(*irq)P((struct nic *, irq_action_t));
 	int		flags;	/* driver specific flags */
 	struct rom_info	*rom_info;	/* -> rom_info from main */
 	unsigned char	*node_addr;
 	unsigned char	*packet;
 	unsigned int	packetlen;
 	unsigned int	ioaddr;
 	unsigned char	irqno;
 	void		*priv_data;	/* driver can hang private data here */
};

static int dummy(struct nic *nic __unused)
{
 	return (0);
}

struct nic	nic =
{
 	{
 		0,				/* dev.disable */
 		{
			0,
 			0,
 			PCI_BUS_TYPE,
 		},				/* dev.devid */
 		0,				/* index */
 		0,				/* type */
 		PROBE_FIRST,			/* how_pobe */
 		PROBE_NONE,			/* to_probe */
 		0,				/* failsafe */
 		0,				/* type_index */
 		{},				/* state */
 	},
 	(int (*)(struct nic *, int))dummy,      /* poll */
 	(void (*)(struct nic *, const char *,
 		unsigned int, unsigned int,
 		const char *))dummy,		/* transmit */
 	(void (*)(struct nic *, irq_action_t))dummy, /* irq */
 	0,					/* flags */
 	&rom,					/* rom_info */
 	arptable[ARP_CLIENT].node,		/* node_addr */
 	packet + ETH_DATA_ALIGN,		/* packet */
 	0,					/* packetlen */
 	0,			/* ioaddr */
 	0,			/* irqno */
 	NULL,					/* priv_data */
};

static const char *driver_name[] = {
	"nic", 
	"disk", 
	"floppy",
};

int probe(struct dev *dev)
{
	const char *type_name;
	type_name = "";
	if ((dev->type >= 0) && 
		((unsigned)dev->type < sizeof(driver_name)/sizeof(driver_name[0]))) {
		type_name = driver_name[dev->type];
	}
	if (dev->how_probe == PROBE_FIRST) {
		dev->to_probe = PROBE_PCI;
		memset(&dev->state, 0, sizeof(dev->state));
	}
	if (dev->to_probe == PROBE_PCI) {
		dev->how_probe = pci_probe(dev, type_name);
	}
	
	return dev->how_probe;
}

int like_eth_probe(void)
{
 	static int probed = 0;
 	struct dev *dev;
 
 	network_ready = 0;
 	dev = &nic.dev;
 	dev->how_probe = PROBE_FIRST;
 	dev->to_probe = PROBE_PCI;
 	dev->type = NIC_DRIVER;
 	dev->failsafe = 1;
 	//rom = *((struct rom_info *)ROM_INFO_LOCATION);

 	probed = probe(dev);
 
 	return probed;
}

int like_eth_poll(int retrieve)
{
 	return ((*nic.poll)(&nic, retrieve));
}

void like_eth_transmit(const char *d, unsigned int t, unsigned int s, const void *p)
{
	(*nic.transmit)(&nic, d, t, s, p);
}
