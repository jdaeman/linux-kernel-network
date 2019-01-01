#include<linux/kernel.h>
#include<linux/spinlock.h> //for spinlock usage
#include<linux/skbuff.h> //for sk_buff struct & functions
#include<linux/module.h> //for EXPORT_SYMBOL

#define Q_SIZE 64 // can changed 

// /usr/src/linux-3.2.28/kernel/this code file
// it is up to you to modify the system call number and table.

struct sk_buff * skbuf_q[Q_SIZE] = {NULL, }; //circular q
int front = 0;
int rear = 0;
int size = 0;

spinlock_t skbuf_q_lock;
DEFINE_SPINLOCK(skbuf_q_lock);

EXPORT_SYMBOL(skbuf_q_lock);

int skbuf_q_push(const struct sk_buff * skb)
{
	int next, ret;
	
	spin_lock(&skbuf_q_lock);

	if(size == Q_SIZE) //full
		ret = -1; //fail
	else
	{
		next = (rear + 1) % Q_SIZE; //next rear
		skbuf_q[rear] = skb_copy(skb, GFP_ATOMIC);
		if(!skbuf_q[rear]) //fail to mem alloc
                	ret = -1;
		else
		{
			rear = next;
        		size += 1;
			ret = 0;
		}
	}	

	spin_unlock(&skbuf_q_lock);
	return ret; //success
}
EXPORT_SYMBOL(skbuf_q_push);

struct sk_buff * skbuf_q_top(void)
{
	struct sk_buff * ret;

	spin_lock(&skbuf_q_lock);

	if(!size) //empty
		ret = NULL; //fail
	else
		ret = skbuf_q[front]; //success

	spin_unlock(&skbuf_q_lock);
	return ret;
}
EXPORT_SYMBOL(skbuf_q_top);

int skbuf_q_pop(void)
{
	int ret;

	spin_lock(&skbuf_q_lock);

	if(!size) //empty
		ret = -1; //fail
	else
	{
		kfree_skb(skbuf_q[front]);
		front = (front + 1) % Q_SIZE; //next front
		size -= 1;
		ret = 0;
	}

	spin_unlock(&skbuf_q_lock);
	return ret;
}
EXPORT_SYMBOL(skbuf_q_pop);

asmlinkage void * sys_look_pkt(void ** args, int len)
{
	return (void*)size; //return current q length
}
