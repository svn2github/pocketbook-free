#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>

#include <video/fbcon.h>
#include <video/fbcon-mfb.h>
#include <video/fbcon-cfb4.h>
#include <video/fbcon-cfb8.h>
#include <video/fbcon-cfb16.h>

#include "s3c2410fb.h"


void (*s3c2410fb_blank_helper)(int blank);
EXPORT_SYMBOL(s3c2410fb_blank_helper);


static struct s3c2410fb_rgb	rgb_8 = {
	red:	{offset: 0, length: 4, },
	green:	{offset: 0, length: 4, },
	blue:	{offset: 0, length: 4, },
	transp:	{offset: 0, length: 0, },
};

static struct s3c2410fb_rgb	def_rgb_16 = {
    	red:	{offset: 11, length: 5, },
	green:	{offset: 5,  length: 6, },
	blue:	{offset: 0,  length: 5, },
	transp:	{offset: 0,  length: 0, },
};

#if 0
static struct s3c2410fb_rgb	xxx_tft_rgb_16 = {
    	red:	{offset: 11,	length: 5, },
	green:	{offset: 6,	length: 5, },
	blue:	{offset: 1,	length: 5, },
	transp:	{offset: 0,	length: 1, },
};
#else
static struct s3c2410fb_rgb	xxx_tft_rgb_16 = {
    	red:	{offset: 11,	length: 5, },
	green:	{offset: 5,	length: 6, },
	blue:	{offset: 0,	length: 5, },
	transp:	{offset: 0,	length: 0, },
};
#endif

#ifdef CONFIG_S3C2410_SMDK
static struct s3c2410fb_mach_info xxx_stn_info __initdata = {
    	pixclock:	174757,		bpp:		16,
#ifdef CONFIG_FB_S3C2410_EMUL
	xres:		96,
#else
	xres:		240,
#endif
	yres:		320,


	hsync_len   :  5,    vsync_len    :  1,
	left_margin :  7,    upper_margin :  1,
	right_margin:  3,    lower_margin :  3,

	sync:		0,		cmap_static:	1,
	reg : {
		lcdcon1 : LCD1_BPP_16T | LCD1_PNR_TFT | LCD1_CLKVAL(7) ,
		lcdcon2 : LCD2_VBPD(1) | LCD2_VFPD(2) | LCD2_VSPW(1),
		lcdcon3 : LCD3_HBPD(6) | LCD3_HFPD(2),
		lcdcon4 : LCD4_HSPW(4) | LCD4_MVAL(13),
		lcdcon5 : LCD5_FRM565 | LCD5_INVVLINE | LCD5_INVVFRAME | LCD5_HWSWP | LCD5_PWREN,
	},
};
#endif

static inline u_int
chan_to_field(u_int chan, struct fb_bitfield *bf)
{
    chan &= 0xffff;
    chan >>= 16 - bf->length;
    return chan << bf->offset;
}

/*
 * Convert bits-per-pixel to a hardware palette PBS value.
 */
static inline u_int
palette_pbs(struct fb_var_screeninfo *var)
{
   int ret = 0;
   switch (var->bits_per_pixel) {
#ifdef FBCON_HAS_CFB4
      case 4:  ret = 0 << 12; break;
#endif
#ifdef FBCON_HAS_CFB8
      case 8:  ret = 1 << 12; break;
#endif
#ifdef FBCON_HAS_CFB16
      case 16: ret = 2 << 12; break;
#endif
   }
   return ret;
}

static struct s3c2410fb_mach_info * __init
s3c2410fb_get_machine_info(struct s3c2410fb_info *fbi)
{
    struct s3c2410fb_mach_info *inf = NULL;

    inf = &xxx_stn_info;
	fbi->reg = inf->reg;
    fbi->rgb[RGB_16] = &xxx_tft_rgb_16;

    return inf;
}

static inline struct fb_var_screeninfo *get_con_var(struct fb_info *info, int con)
{
    struct s3c2410fb_info *fbi = (struct s3c2410fb_info *)info;
    return (con == fbi->currcon || con == -1) ? &fbi->fb.var : &fb_display[con].var;
}

static inline struct fb_cmap *get_con_cmap(struct fb_info *info, int con)
{
    struct s3c2410fb_info *fbi = (struct s3c2410fb_info *)info;
    return (con == fbi->currcon || con == -1) ? &fbi->fb.cmap : &fb_display[con].cmap;
}

static inline struct display *get_con_display(struct fb_info *info, int con)
{
    struct s3c2410fb_info *fbi = (struct s3c2410fb_info *)info;
    return (con < 0) ? (fbi->fb.disp) : &(fb_display[con]);
}

static int
s3c2410fb_validate_var(struct fb_var_screeninfo *var,
			      struct s3c2410fb_info *fbi)
{
    int ret = -EINVAL;

    if (var->xres < MIN_XRES)
		var->xres = MIN_XRES;
    if (var->yres < MIN_YRES)
		var->yres = MIN_YRES;
    if (var->xres > fbi->max_xres)
		var->xres = fbi->max_xres;
    if (var->yres > fbi->max_yres)
		var->yres = fbi->max_yres;
    var->xres_virtual =
	var->xres_virtual < var->xres ? var->xres : var->xres_virtual;
    var->yres_virtual = 
	var->yres_virtual < var->yres ? var->yres : var->yres_virtual;

    switch(var->bits_per_pixel) {
#ifdef FBCON_HAS_CFB4
	case 4:		ret = 0; break;
#endif
#ifdef FBCON_HAS_CFB8
	case 8:		ret = 0; break;
#endif
#ifdef FBCON_HAS_CFB16
	case 16:	ret = 0; break;
#endif
	default:
			break;
    }

    return ret;
}
static int
s3c2410fb_setpalettereg(u_int regno, u_int red, u_int green, u_int blue,
			u_int trans, struct fb_info *info)
{
    struct s3c2410fb_info *fbi = (struct s3c2410fb_info *)info;
    u_int val, ret = 1;

    if (regno < fbi->palette_size) {
#ifndef CONFIG_S3C2400_GAMEPARK
	val = ((red >> 4) & 0xf00);
	val |= ((green >> 8) & 0x0f0);
	val |= ((blue >> 12) & 0x00f);
#else
	val = ((blue >> 16) & 0x001f);
    val |= ((green >> 11) & 0x07e0);
    val |= ((red >> 5) & 0x0f800);
	val |= 1; /* intensity bit */
#endif	
	if (regno == 0)
	    val |= palette_pbs(&fbi->fb.var);

	fbi->palette_cpu[regno] = val;
	ret = 0;
    }
    return ret;
}


static int
s3c2410fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
		    u_int trans, struct fb_info *info)
{
    struct s3c2410fb_info *fbi = (struct s3c2410fb_info *)info;
    struct display *disp = get_con_display(info, fbi->currcon);
    u_int var;
    int ret = 1;

    if (disp->inverse) {
	red 	= 0xffff - red;
	green 	= 0xffff - green;
	blue	= 0xffff - blue;
    }

    if (fbi->fb.var.grayscale) 
	red = green = blue = (19595 * red + 38470 * green + 7471 * blue) >> 16;

    switch (fbi->fb.disp->visual) {
	case FB_VISUAL_TRUECOLOR:
	    if (regno < 16) {
		u16 *pal = fbi->fb.pseudo_palette;

		var = chan_to_field(red, &fbi->fb.var.red);
		var |= chan_to_field(green, &fbi->fb.var.green);
		var |= chan_to_field(blue, &fbi->fb.var.blue);

		pal[regno] = var;
		ret = 0;
	    }
	    break;

	case FB_VISUAL_STATIC_PSEUDOCOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
	    ret = s3c2410fb_setpalettereg(regno, red, green, blue, trans, info);
	    break;
    }

    return ret;
}

static int s3c2410fb_activate_var(struct fb_var_screeninfo *var, struct s3c2410fb_info *fbi)
{
    struct s3c2410fb_lcd_reg new_regs;
    u_int half_screen_size, yres;
    u_long flags;

    unsigned long VideoPhysicalTemp = fbi->screen_dma;

    save_flags_cli(flags);

    new_regs.lcdcon1 = fbi->reg.lcdcon1 & ~LCD1_ENVID;

    new_regs.lcdcon2 = (fbi->reg.lcdcon2 & ~LCD2_LINEVAL_MSK) 
						| LCD2_LINEVAL(var->yres - 1);

	/* TFT LCD only ! */
    new_regs.lcdcon3 = (fbi->reg.lcdcon3 & ~LCD3_HOZVAL_MSK)
						| LCD3_HOZVAL(var->xres - 1);

    new_regs.lcdcon4 = fbi->reg.lcdcon4;
    new_regs.lcdcon5 = fbi->reg.lcdcon5;

    new_regs.lcdsaddr1 = 
		LCDADDR_BANK(((unsigned long)VideoPhysicalTemp >> 22))
		| LCDADDR_BASEU(((unsigned long)VideoPhysicalTemp >> 1));

	/* 16bpp */
    new_regs.lcdsaddr2 = LCDADDR_BASEL( 
		((unsigned long)VideoPhysicalTemp + (var->xres * 2 * (var->yres/*-1*/)))
		>> 1);

    new_regs.lcdsaddr3 = LCDADDR_OFFSET(0) | (LCDADDR_PAGE(var->xres) /*>> 1*/);

    yres = var->yres;

    half_screen_size = var->bits_per_pixel;
    half_screen_size = half_screen_size * var->xres * var->yres / 16;

	fbi->reg.lcdcon1 = new_regs.lcdcon1;
	fbi->reg.lcdcon2 = new_regs.lcdcon2;
	fbi->reg.lcdcon3 = new_regs.lcdcon3;
	fbi->reg.lcdcon4 = new_regs.lcdcon4;
	fbi->reg.lcdcon5 = new_regs.lcdcon5;
	fbi->reg.lcdsaddr1 = new_regs.lcdsaddr1;
	fbi->reg.lcdsaddr2 = new_regs.lcdsaddr2;
	fbi->reg.lcdsaddr3 = new_regs.lcdsaddr3;

    LCDCON1 = fbi->reg.lcdcon1;
    LCDCON2 = fbi->reg.lcdcon2;
    LCDCON3 = fbi->reg.lcdcon3;
    LCDCON4 = fbi->reg.lcdcon4;
    LCDCON5 = fbi->reg.lcdcon5;
    LCDADDR1 = fbi->reg.lcdsaddr1;
    LCDADDR2 = fbi->reg.lcdsaddr2;
    LCDADDR3 = fbi->reg.lcdsaddr3;

#if defined(CONFIG_S3C2410_SMDK) && !defined(CONFIG_SMDK_AIJI)
    LCDLPCSEL = 0x2;	
#elif defined(CONFIG_S3C2410_SMDK) && defined(CONFIG_SMDK_AIJI) 
    LCDLPCSEL = 0x7;
#endif
   
    TPAL = 0;
    LCDCON1 |= LCD1_ENVID;

#if 0
	{
	printk("con1 = 0x%08lx\n", LCDCON1);
	printk("con2 = 0x%08lx\n", LCDCON2);

	printk("con3 = 0x%08lx\n", LCDCON3);
	printk("con4 = 0x%08lx\n", LCDCON4);
	printk("con5 = 0x%08lx\n", LCDCON5);
	printk("addr1 = 0x%08lx\n", LCDADDR1);
	printk("addr2 = 0x%08lx\n", LCDADDR2);
	printk("addr3 = 0x%08lx\n", LCDADDR3);
	}
#endif

    restore_flags(flags);

    return 0;
}


static inline void s3c2410fb_set_truecolor(u_int is_true_color)
{
    if (is_true_color) {
    }
    else {
    }
}

    
static void
s3c2410fb_hw_set_var(struct fb_var_screeninfo *var, struct s3c2410fb_info *fbi)
{
    u_long palette_mem_size;

    fbi->palette_size = var->bits_per_pixel == 8 ? 256 : 16;

    palette_mem_size = fbi->palette_size * sizeof(u16);

    fbi->palette_cpu = (u16 *)(fbi->map_cpu + PAGE_SIZE - palette_mem_size);
    fbi->palette_dma = fbi->map_dma + PAGE_SIZE - palette_mem_size;

    fb_set_cmap(&fbi->fb.cmap, 1, s3c2410fb_setcolreg, &fbi->fb);

    s3c2410fb_set_truecolor(var->bits_per_pixel >= 16);

    s3c2410fb_activate_var(var, fbi);

    fbi->palette_cpu[0] = (fbi->palette_cpu[0] &
				0xcfff) | palette_pbs(var);
}



static int
s3c2410fb_set_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
    struct s3c2410fb_info *fbi = (struct s3c2410fb_info *)info;
    struct fb_var_screeninfo *dvar = get_con_var(&fbi->fb, con);
    struct display *display = get_con_display(&fbi->fb, con);
    int err, chgvar = 0, rgbidx;

    
    err = s3c2410fb_validate_var(var, fbi);

    if (err) 
	return err;

    if (var->activate & FB_ACTIVATE_TEST)
	return 0;
    if ((var->activate & FB_ACTIVATE_MASK) != FB_ACTIVATE_NOW)
	return -EINVAL;

    if (dvar->xres != var->xres)
	chgvar = 1;
    if (dvar->yres != var->yres)
	chgvar = 1;
    if (dvar->xres_virtual != var->xres_virtual)
	chgvar = 1;
    if (dvar->yres_virtual != var->yres_virtual)
	chgvar = 1;
    if (dvar->bits_per_pixel != var->bits_per_pixel)
	chgvar = 1;
    if (con < 0)
	chgvar = 0;

    switch (var->bits_per_pixel) {
#ifdef FBCON_HAS_CFB4
	case 4:
	    	if (fbi->cmap_static)
		    display->visual = FB_VISUAL_STATIC_PSEUDOCOLOR;
		else
		    display->visual = FB_VISUAL_PSEUDOCOLOR;
		display->line_length 	= var->xres / 2;
		display->dispsw		= &fbcon_cfb4;
		rgbidx			= RGB_8;
		break;
#endif
#ifdef	FBCON_HAS_CFB8
	case 8:
		if (fbi->cmap_static)
		    display->visual = FB_VISUAL_STATIC_PSEUDOCOLOR;
		else
		    display->visual = FB_VISUAL_PSEUDOCOLOR;
		display->line_length 	= var->xres;
	    	display->dispsw		= &fbcon_cfb8;
		rgbidx			= RGB_8;
		break;
#endif
#ifdef FBCON_HAS_CFB16
	case 16:
		display->visual		= FB_VISUAL_TRUECOLOR;
#ifdef CONFIG_FB_S3C2410_EMUL
		display->line_length    = 240*2;
#else
		display->line_length	= var->xres * 2;
#endif
		display->dispsw		= &fbcon_cfb16;
		display->dispsw_data	= fbi->fb.pseudo_palette;
		rgbidx			= RGB_16;
		break;
#endif
	default:
		rgbidx = 0;
		display->dispsw = &fbcon_dummy;
		break;
    }

    display->screen_base 	= fbi->screen_cpu;
    display->next_line		= display->line_length;
    display->type		= fbi->fb.fix.type;
    display->type_aux		= fbi->fb.fix.type_aux;
    display->ypanstep		= fbi->fb.fix.ypanstep;
    display->ywrapstep		= fbi->fb.fix.ywrapstep;
    display->can_soft_blank	= 0;
    display->inverse		= fbi->cmap_inverse;

    *dvar			= *var;
    dvar->activate		&= ~FB_ACTIVATE_ALL;

    dvar->red			= fbi->rgb[rgbidx]->red;
    dvar->green			= fbi->rgb[rgbidx]->green;
    dvar->blue			= fbi->rgb[rgbidx]->blue;
    dvar->transp		= fbi->rgb[rgbidx]->transp;

    display->var = *dvar;

    if (var->activate & FB_ACTIVATE_ALL)
	fbi->fb.disp->var = *dvar;

    if (chgvar && info && fbi->fb.changevar)
	fbi->fb.changevar(con);

    if (con != fbi->currcon)
	return 0;

    s3c2410fb_hw_set_var(dvar, fbi);

    return 0;
}

static int
__do_set_cmap(struct fb_cmap *cmap, int kspc, int con,
	      struct fb_info *info)
{
    struct s3c2410fb_info *fbi = (struct s3c2410fb_info *)info;
    struct fb_cmap *dcmap = get_con_cmap(info, con);
    int err = 0;

    if (con == -1)
	con = fbi->currcon;

    if (con >= 0)
	err = fb_alloc_cmap(&fb_display[con].cmap, fbi->palette_size, 0);

    if (!err && con == fbi->currcon)
	err = fb_set_cmap(cmap, kspc, s3c2410fb_setcolreg, info);

    if (!err)
	fb_copy_cmap(cmap, dcmap, kspc ? 0 : 1);

    return err;
}

static int
s3c2410fb_set_cmap(struct fb_cmap *cmap, int kspc, int con,
		   struct fb_info *info)
{
    struct display *disp = get_con_display(info, con);

    if (disp->visual == FB_VISUAL_TRUECOLOR ||
        disp->visual == FB_VISUAL_STATIC_PSEUDOCOLOR)
	return -EINVAL;

    return __do_set_cmap(cmap, kspc, con, info);
}


static int
s3c2410fb_get_fix(struct fb_fix_screeninfo *fix, int con, struct fb_info *info)
{
    struct display *display = get_con_display(info, con);

    *fix = info->fix;

    fix->line_length = display->line_length;
    fix->visual      = display->visual;
    return 0;
}

static int
s3c2410fb_get_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
    *var = *get_con_var(info, con);
    return 0;
}

static int
s3c2410fb_get_cmap(struct fb_cmap *cmap, int kspc, int con, struct fb_info *info)
{
    struct fb_cmap *dcmap = get_con_cmap(info, con);
    fb_copy_cmap(dcmap, cmap, kspc ? 0 : 2);
    return 0;
}

#ifdef CONFIG_PM
static int
s3c2410fb_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
               unsigned long arg, int con, struct fb_info *info) {
#ifdef CONFIG_MIZI
    if (mz_pm_ops.fb_ioctl == NULL)
      return -EINVAL;
    return (*(mz_pm_ops.fb_ioctl))(inode, file, cmd, arg, PROC_CONSOLE(info), info);
#endif /* CONFIG_MIZI */
}
#endif /* CONFIG_PM */

static struct fb_ops s3c2410fb_ops = {
	owner:		THIS_MODULE,
	fb_get_fix:	s3c2410fb_get_fix,
	fb_get_var:	s3c2410fb_get_var,
	fb_set_var:	s3c2410fb_set_var,
	fb_get_cmap:	s3c2410fb_get_cmap,
	fb_set_cmap:	s3c2410fb_set_cmap,
#ifdef CONFIG_PM
    fb_ioctl:   s3c2410fb_ioctl,
#endif /* CONFIG_PM */
};

static int s3c2410fb_switch(int con, struct fb_info *info)
{
    struct s3c2410fb_info *fbi = (struct s3c2410fb_info *)info;
    struct display *disp;
    struct fb_cmap *cmap;

    if (con == fbi->currcon)
	return 0;

    if (fbi->currcon >= 0) {
	disp = fb_display + fbi->currcon;

	disp->var = fbi->fb.var;

	if (disp->cmap.len)
	    fb_copy_cmap(&fbi->fb.cmap, &disp->cmap, 0);

    }

    fbi->currcon = con;
    disp = fb_display + con;

    fb_alloc_cmap(&fbi->fb.cmap, 256, 0);

    if (disp->cmap.len)
	cmap = &disp->cmap;
    else
	cmap = fb_default_cmap(1 << disp->var.bits_per_pixel);

    fb_copy_cmap(cmap, &fbi->fb.cmap, 0);

    fbi->fb.var = disp->var;
    fbi->fb.var.activate = FB_ACTIVATE_NOW;

    s3c2410fb_set_var(&fbi->fb.var, con, info);

    return 0;
}

#ifdef CONFIG_PM
/*
 * Power management hook. Note that we won't be called from IRQ context,
 * unlike the blank functions above, so we may sleep
 */
static int s3c2410_pm_callback(struct pm_dev *pm_dev, pm_request_t req, void *data)
{
	struct s3c2410fb_info *fbi = pm_dev->data;
	u_long flags;

	//printk("pm_callback: %d\n", req);

	if (req == PM_SUSPEND) {
		/* disable LCD controller */
		LCDCON1 &= ~(1 << 0);
	} else if (req == PM_RESUME) {
		/* reinitialize LCD controllers and GPIOs */
		save_flags_cli(flags);

		LCDCON1 = fbi->reg.lcdcon1;
		LCDCON2 = fbi->reg.lcdcon2;
		LCDCON3 = fbi->reg.lcdcon3;
		LCDCON4 = fbi->reg.lcdcon4;
		LCDCON5 = fbi->reg.lcdcon5;
		LCDADDR1 = fbi->reg.lcdsaddr1;
		LCDADDR2 = fbi->reg.lcdsaddr2;
		LCDADDR3 = fbi->reg.lcdsaddr3;

#if defined(CONFIG_S3C2410_SMDK) && !defined(CONFIG_SMDK_AIJI)
		LCDLPCSEL = 0x2;	
#elif defined(CONFIG_S3C2410_SMDK) && defined(CONFIG_SMDK_AIJI)
		LCDLPCSEL = 0x7;
#endif

		TPAL = 0;
		LCDCON1 |= LCD1_ENVID;

		restore_flags(flags);
	}
	//printk("done\n");
	return 0;
}
#endif

static int __init s3c2410fb_map_video_memory(struct s3c2410fb_info *fbi)
{
    fbi->map_size = PAGE_ALIGN(fbi->fb.fix.smem_len + PAGE_SIZE);
    fbi->map_cpu = consistent_alloc(GFP_KERNEL, fbi->map_size,
	    			    &fbi->map_dma);

    if (fbi->map_cpu) {
		fbi->screen_cpu = fbi->map_cpu + PAGE_SIZE;
		fbi->screen_dma = fbi->map_dma + PAGE_SIZE;
		fbi->fb.fix.smem_start = fbi->screen_dma;
    }

    return fbi->map_cpu ? 0 : -ENOMEM;
}

static int s3c2410fb_updatevar(int con, struct fb_info *info)
{
    return 0;
}

static void s3c2410fb_blank(int blank, struct fb_info *info)
{
}

static struct fb_monspecs monspecs __initdata = {
    30000, 70000, 50, 65, 0
};

static struct s3c2410fb_info * __init s3c2410fb_init_fbinfo(void)
{
    struct s3c2410fb_mach_info *inf;
    struct s3c2410fb_info *fbi;

    fbi = kmalloc(sizeof(struct s3c2410fb_info) + sizeof(struct display) +
	    	  sizeof(u16)*16, GFP_KERNEL);

    if (!fbi)
	return NULL;

    memset(fbi, 0, sizeof(struct s3c2410fb_info) + sizeof(struct display));

    fbi->currcon		= -1;

    strcpy(fbi->fb.fix.id, S3C2410_NAME);

    fbi->fb.fix.type		= FB_TYPE_PACKED_PIXELS;
    fbi->fb.fix.type_aux	= 0;
    fbi->fb.fix.xpanstep	= 0;
    fbi->fb.fix.ypanstep	= 0;
    fbi->fb.fix.ywrapstep	= 0;
    fbi->fb.fix.accel		= FB_ACCEL_NONE;

    fbi->fb.var.nonstd		= 0;
    fbi->fb.var.activate	= FB_ACTIVATE_NOW;
    fbi->fb.var.height		= -1;
    fbi->fb.var.width		= -1;
    fbi->fb.var.accel_flags	= 0;
    fbi->fb.var.vmode		= FB_VMODE_NONINTERLACED;

    strcpy(fbi->fb.modename, S3C2410_NAME);
    strcpy(fbi->fb.fontname, "Acorn8x8");

    fbi->fb.fbops		= &s3c2410fb_ops;
    fbi->fb.changevar		= NULL;
    fbi->fb.switch_con		= s3c2410fb_switch;
    fbi->fb.updatevar		= s3c2410fb_updatevar;
    fbi->fb.blank		= s3c2410fb_blank;
    fbi->fb.flags		= FBINFO_FLAG_DEFAULT;
    fbi->fb.node		= -1;
    fbi->fb.monspecs		= monspecs;
    fbi->fb.disp		= (struct display *)(fbi + 1);
    fbi->fb.pseudo_palette	= (void *)(fbi->fb.disp + 1);

    fbi->rgb[RGB_8]		= &rgb_8;
    fbi->rgb[RGB_16]		= &def_rgb_16;

    inf	= s3c2410fb_get_machine_info(fbi);

#ifdef CONFIG_FB_S3C2410_EMUL
	fbi->max_xres	 = 240;
#else
    fbi->max_xres		= inf->xres;
#endif
    fbi->fb.var.xres		= inf->xres;
    fbi->fb.var.xres_virtual	= inf->xres;
    fbi->max_yres		= inf->yres;
    fbi->fb.var.yres		= inf->yres;
    fbi->fb.var.yres_virtual	= inf->yres;
    fbi->max_bpp		= inf->bpp;
    fbi->fb.var.bits_per_pixel  = inf->bpp;
    fbi->fb.var.pixclock	= inf->pixclock;
    fbi->fb.var.hsync_len	= inf->hsync_len;
    fbi->fb.var.left_margin	= inf->left_margin;
    fbi->fb.var.right_margin	= inf->right_margin;
    fbi->fb.var.vsync_len	= inf->vsync_len;
    fbi->fb.var.upper_margin	= inf->upper_margin;
    fbi->fb.var.lower_margin	= inf->lower_margin;
    fbi->fb.var.sync		= inf->sync;
    fbi->fb.var.grayscale	= inf->cmap_grayscale;
    fbi->cmap_inverse		= inf->cmap_inverse;
    fbi->cmap_static		= inf->cmap_static;
    fbi->fb.fix.smem_len	= fbi->max_xres * fbi->max_yres *
				  fbi->max_bpp / 8;
    return fbi;
}

void s3c2410_lcd_init(void)
{
    GPDCON = 0xaaaaaaaa;

#ifdef CONFIG_S3C2410_SMDK
    GPCCON = 0xaaaaaaaa;
	set_gpio_ctrl(GPIO_G4 | GPIO_PULLUP_EN | GPIO_MODE_LCD_PWRDN);
#endif

#if defined(CONFIG_MIZI) && defined(CONFIG_PM)
    if (mz_pm_ops.blank_helper != NULL)
        (*(mz_pm_ops.blank_helper))(MZ_BLANK_ON);
#endif	
}

int __init s3c2410fb_init(void)
{
    struct s3c2410fb_info *fbi;
    int ret;

    fbi = s3c2410fb_init_fbinfo();
    ret = -ENOMEM;
    if (!fbi)
	goto failed;

    ret = s3c2410fb_map_video_memory(fbi);
    if (ret)
	goto failed;

    s3c2410_lcd_init();
    s3c2410fb_set_var(&fbi->fb.var, -1, &fbi->fb);

    ret = register_framebuffer(&fbi->fb);
   if (ret < 0)
      goto failed;

#ifdef CONFIG_PM
	/*
	 * Note that console registers this as well, but we want to
	 * power donw the display prior to sleeping
	 */
	fbi->pm = pm_register(PM_DEBUG_DEV, PM_SYS_VGA, s3c2410_pm_callback);
	if (fbi->pm)
		fbi->pm->data = fbi;
#endif

   /* enable the LCD controller) */
	printk("Installed S3C2410 frame buffer\n");

    MOD_INC_USE_COUNT ;
    return 0;

failed:
    if (fbi)
	kfree(fbi);
    return ret;
}

int __init s3c2410fb_setup(char *options)
{
    return 0;
}

#ifdef MODULE
module_init(s3c2410fb_init);
#endif
