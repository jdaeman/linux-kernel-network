#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/highmem.h>
#include<linux/skbuff.h>
#include<linux/spinlock.h>
#include<linux/if_ether.h>
#include<linux/ip.h>
#include<linux/tcp.h>
#include<linux/udp.h>

#include"lookpkt.h"

#define __NR_look_pkt 349 //my system call number

extern spinlock_t skbuf_q_lock;
extern struct sk_buff * skbuf_q_top(void);
extern int skbuf_q_pop(void);

void ** sys_call_table = (void**)SYS_CALL_TABLE; //from the Makefile
asmlinkage void * (*org_look_pkt) (void ** args, int len);

asmlinkage void * hooked_look_pkt(void ** args, int len);
void get_eth_h(void ** args, int len);
void get_net_h(void ** args, int len);
void get_tcp_h(void ** args, int len);
void get_udp_h(void ** args, int len);
void get_data(void ** args, int len);

void make_rw(void * address)
{
	unsigned int level;
	pte_t * pte = lookup_address((unsigned long)address, &level);
	if(pte->pte & ~_PAGE_RW)
		pte->pte |= _PAGE_RW;
}

void make_ro(void * address)
{
	unsigned int level;
	pte_t * pte = lookup_address((unsigned long)address, &level);
	pte->pte = pte->pte & ~_PAGE_RW;
}

int start(void)
{
	make_rw(sys_call_table);
	org_look_pkt = sys_call_table[__NR_look_pkt];
	sys_call_table[__NR_look_pkt] = hooked_look_pkt;
	return 0;
}

void end(void)
{
	sys_call_table[__NR_look_pkt] = org_look_pkt;
	make_ro(sys_call_table);
}

asmlinkage void * hooked_look_pkt(void ** args, int len)
{
	int op;
	op = (int)args[0];
	
	switch(op)
	{
	case GET_SIZE:
		return (void*)org_look_pkt(args, len);
	case REMOVE_ONE:
		skbuf_q_pop();
		return (void*)0;
	case GET_ETH_H:
		get_eth_h(args, len);
		return (void*)0;
	case GET_NET_H:
		get_net_h(args, len);
		return (void*)0;
	case GET_TCP_H:
		get_tcp_h(args, len);
		return (void*)0;
	case GET_UDP_H:
		get_udp_h(args, len);
		return (void*)0;
	case GET_DATA:
		get_data(args, len);
		return (void*)0;		
	}
	
	return (void*)(-1);
}

void get_eth_h(void ** args, int len)
{
	struct sk_buff * pkt;
	char buf[512];

	memset(buf, 0, 512);
	pkt = skbuf_q_top();
	if(pkt)
	{
		struct ethhdr * eh = (struct ethhdr *)(pkt->mac_header);
		sprintf(buf, "soruce: %pM, dest: %pM, protocol: %#04x", 
				eh->h_source, eh->h_dest, ntohs(eh->h_proto));
		strcpy((char*)args[1], buf);
	}
}

void get_net_h(void ** args, int len)
{
	struct sk_buff * pkt;
	char buf[512];	
	
	memset(buf, 0, 512);
	pkt = skbuf_q_top();
	if(pkt)
	{
		struct iphdr * ih = (struct iphdr*)(pkt->network_header);
		sprintf(buf, "source: %pI4, dest: %pI4, protocol: %#02X",
				&ih->saddr, &ih->daddr, ih->protocol);
		strcpy((char*)args[1], buf);
	}	
}

void get_tcp_h(void ** args, int len)
{
        struct sk_buff * pkt;
        char buf[512];

        memset(buf, 0, 512);
        pkt = skbuf_q_top();
        if(pkt)
	{
		struct tcphdr * th = (struct tcphdr*)(pkt->transport_header);
		sprintf(buf, "source: %d, dest: %d, ack: %d, ack_seq: %d",
				ntohs(th->source), ntohs(th->dest), ntohl(th->ack), ntohl(th->ack_seq));
		strcpy((char*)args[1], buf);	
	}
}

void get_udp_h(void ** args, int len)
{
	struct sk_buff * pkt;
	char buf[512];

        memset(buf, 0, 512);
        pkt = skbuf_q_top();
        if(pkt)
        {
                struct udphdr * uh = (struct udphdr*)(pkt->transport_header);
		sprintf(buf, "source: %d, dest: %d, len: %d\n",
				ntohs(uh->source), ntohs(uh->dest), ntohs(uh->len));
		strcpy((char*)args[1], buf);	
        }
}

void get_data(void ** args, int len)
{
	struct sk_buff * pkt;
	char buf[512];
	int buf_idx = 0;
	unsigned char * data_ptr = NULL;

	pkt = skbuf_q_top();
	if(pkt)
	{
		unsigned short proto = 0;

		if(pkt->data == pkt->mac_header)
			skb_pull(pkt, sizeof(struct ethhdr));
		if(pkt->data == pkt->network_header)
		{
			proto = ((struct iphdr *)pkt->data)->protocol;
			skb_pull(pkt, sizeof(struct iphdr));
		}
		if(pkt->data == pkt->transport_header)
		{
			if(proto == IPPROTO_TCP)
				skb_pull(pkt, sizeof(struct tcphdr));
			else if(proto == IPPROTO_UDP)
				skb_pull(pkt, sizeof(struct udphdr));
			else 
			{	//empty
			}
		}
		
		printk("in here1\n");
	
		data_ptr = pkt->data;
		while(data_ptr != pkt->tail)
		{
			memset(buf, 0, 512);
			for(buf_idx = 0; buf_idx < 511; buf_idx++)
			{
				buf[buf_idx] = *(data_ptr++);
				if(data_ptr == pkt->tail)
					break;
			}
			strcat((char*)args[1], buf);
		}
		
		//encapsulation....
		//in later...
	}
}




module_init(start);
module_exit(end);
MODULE_LICENSE("GPL");
