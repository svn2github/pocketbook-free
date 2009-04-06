/*
 * drivers/pcmcia/sa1100_cerf.c
 *
 * PCMCIA implementation routines for CerfBoard
 * Based off the Assabet.
 *
 */
#include <linux/kernel.h>
#include <linux/sched.h>

#include <asm/hardware.h>
#include <asm/irq.h>

#ifdef CONFIG_SA1100_CERF
include "sa1100_generic.h"
#else
#include <asm/arch/pcmcia.h>
#endif

#include "sa1100_cerf.h"

/*
 * Set this to zero to remove all the debug statements via
 * dead code elimination
 */
//#define DEBUGGING       1

#if DEBUGGING
static unsigned int pcmcia_debug = DEBUGGING;
#else
#define pcmcia_debug 0     /* gcc will remove all the debug code for us */
#endif

static struct irqs {
	int irq;
	unsigned int gpio;
	const char *str;
} irqs[] = {
	{ PCMCIA_IRQ_CF_CD,   PCMCIA_GPIO_CF_CD_EDGE,   "CF_CD"   },
	{ PCMCIA_IRQ_CF_BVD2, PCMCIA_GPIO_CF_BVD2_EDGE, "CF_BVD2" },
	{ PCMCIA_IRQ_CF_BVD1, PCMCIA_GPIO_CF_BVD1_EDGE, "CF_BVD1" }
};

static int cerf_pcmcia_init(struct pcmcia_init *init)
{
	int i, res;

	if( pcmcia_debug)
		printk( KERN_INFO "cerf_pcmcia_init: enter\n");

	cerf_pcmcia_set_gpio_direction();

	set_GPIO_IRQ_edge( PCMCIA_GPIO_CF_IRQ_EDGE, GPIO_FALLING_EDGE );

	for (i = 0; i < ARRAY_SIZE(irqs); i++) {

		set_GPIO_IRQ_edge(irqs[i].gpio, GPIO_BOTH_EDGES);

		res = request_irq(irqs[i].irq, init->handler, SA_INTERRUPT,
				irqs[i].str, NULL);
		if (res)
			goto irq_err;
	}

	printk( KERN_INFO "PCMCIA for Cerf: OK\n");

	return CERF_SOCKET+1; /* last socket used +1 */

irq_err:
	printk(KERN_ERR "%s: Request for IRQ%d failed\n", 
		__FUNCTION__, irqs[i].irq);

	while (i--)
		free_irq(irqs[i].irq, NULL);

	return -1;
}

static int cerf_pcmcia_shutdown(void)
{
	int i;
	if( pcmcia_debug)
		printk( KERN_INFO "cerf_pcmcia_shutdown: enter\n");

	for (i = 0; i < ARRAY_SIZE(irqs); i++)
		free_irq(irqs[i].irq, NULL);

	return 0;
}

static int cerf_pcmcia_socket_state(struct pcmcia_state_array *state_array)
{
	int i = CERF_SOCKET;

	if( pcmcia_debug > 3)
		printk( KERN_INFO "cerf_pcmcia_socket_state: i=%d, size=%d\n",
				i, state_array->size);

        memset(state_array->state, 0,
                        (state_array->size)*sizeof(struct pcmcia_state));

	state_array->state[i].detect = cerf_pcmcia_level_detect();
	state_array->state[i].ready  = cerf_pcmcia_level_ready();
	state_array->state[i].bvd1   = cerf_pcmcia_level_bvd1();
	state_array->state[i].bvd2   = cerf_pcmcia_level_bvd2();
	state_array->state[i].wrprot=0;
	state_array->state[i].vs_3v=1;
	state_array->state[i].vs_Xv=0;

	if( pcmcia_debug > 3)
		printk( KERN_INFO "cerf_pcmcia_socket_state: "
			"detect=%d ready=%d bvd1=%d bvd2=%d\n", 
			state_array->state[i].detect,
			state_array->state[i].ready,
			state_array->state[i].bvd1,
			state_array->state[i].bvd2);

	return 1;
}

static int cerf_pcmcia_get_irq_info(struct pcmcia_irq_info *info){

	if( pcmcia_debug)
		printk( KERN_INFO "cerf_pcmcia_get_irq_info: "
				"sock=%d\n", info->sock);

	if(info->sock>1) return -1;

	if (info->sock == CERF_SOCKET)
		info->irq=PCMCIA_IRQ_CF_IRQ;

	if( pcmcia_debug)
		printk( KERN_INFO "cerf_pcmcia_get_irq_info: irq=%d\n",info->irq);

	return 0;
}

static int cerf_pcmcia_configure_socket(const struct pcmcia_configure
					*configure)
{
	if( pcmcia_debug)
		printk( KERN_INFO "cerf_pcmcia_configure_socket:"
			"sock=%d vcc=%d reset=%d\n",
			configure->sock, configure->vcc, configure->reset);

	if(configure->sock>1)
		return -1;

	if (configure->sock != CERF_SOCKET)
		return 0;

	switch(configure->vcc){
		case 0:
			break;

		case 50:
		case 33:
#if defined(CONFIG_SA1100_CERF_CPLD)
                        PCMCIA_GPDR |= PCMCIA_PWR_SHUTDOWN;
                        PCMCIA_GPCR |= PCMCIA_PWR_SHUTDOWN;
#endif
                        /* voltage selected automatically */
			break;

		default:
			printk(KERN_ERR "%s(): unrecognized Vcc %u\n", 
				__FUNCTION__, configure->vcc);
			return -1;
	}

	if(configure->reset)
	{
		PCMCIA_GPSR = PCMCIA_GPIO_CF_RESET_MASK;
	}
	else
	{
		PCMCIA_GPCR = PCMCIA_GPIO_CF_RESET_MASK;
	}

	return 0;
}

#ifdef CONFIG_SA1100_CERF
static int cerf_pcmcia_socket_init(int sock)
{
  int i;

  if( pcmcia_debug)
      printk( KERN_INFO "cerf_pcmcia_socket_init: sock=%d\n",sock);

  if (sock == CERF_SOCKET)
    for (i = 0; i < ARRAY_SIZE(irqs); i++)
      set_GPIO_IRQ_edge(irqs[i].gpio, GPIO_BOTH_EDGES);

  return 0;
}

static int cerf_pcmcia_socket_suspend(int sock)
{
  int i;

  if( pcmcia_debug)
      printk( KERN_INFO "cerf_pcmcia_socket_suspend: sock=%d\n",sock);

  if (sock == CERF_SOCKET)
    for (i = 0; i < ARRAY_SIZE(irqs); i++)
      set_GPIO_IRQ_edge(irqs[i].gpio, GPIO_NO_EDGES);

  return 0;
}
#endif

struct pcmcia_low_level cerf_pcmcia_ops = { 
  init:			cerf_pcmcia_init,
  shutdown:		cerf_pcmcia_shutdown,
  socket_state:		cerf_pcmcia_socket_state,
  get_irq_info:		cerf_pcmcia_get_irq_info,
  configure_socket:	cerf_pcmcia_configure_socket,

#ifdef CONFIG_SA1100_CERF
  socket_init:		cerf_pcmcia_socket_init,
  socket_suspend:	cerf_pcmcia_socket_suspend,
#endif
};

