struct s3c2410fb_rgb {
    struct fb_bitfield		red;
    struct fb_bitfield		green;
    struct fb_bitfield		blue;
    struct fb_bitfield		transp;
};

struct s3c2410fb_lcd_reg {
    unsigned long lcdcon1;
    unsigned long lcdcon2;
    unsigned long lcdcon3;
    unsigned long lcdcon4;
    unsigned long lcdcon5;
    unsigned long lcdsaddr1;
    unsigned long lcdsaddr2;
    unsigned long lcdsaddr3;
};

struct s3c2410fb_mach_info {
    u_long		pixclock;

    u_short		xres;
    u_short		yres;

    u_char		bpp;
    u_char		hsync_len;
    u_char		left_margin;
    u_char		right_margin;

    u_char		vsync_len;
    u_char		upper_margin;
    u_char		lower_margin;
    u_char		sync;

    u_int		cmap_grayscale:1,
    			cmap_inverse:1,
			cmap_static:1,
		        unused:29;
    u_int		state;

	struct s3c2410fb_lcd_reg reg;
#if 0
    unsigned long redlut;
    unsigned long greenlut;
    unsigned long bluelut;

    unsigned long dithmode;
    unsigned long tpal;
#endif
};

#define RGB_8	(0)
#define RGB_16	(1)
#define RGB_24  (2)

#define NR_RGB	3

struct s3c2410fb_info {
    struct fb_info		fb;

    signed int			currcon;

    struct s3c2410fb_rgb	*rgb[NR_RGB];

    u_int			max_bpp;
    u_int			max_xres;
    u_int			max_yres;

    dma_addr_t			map_dma;
    u_char *			map_cpu;
    u_int			map_size;

    u_char *			screen_cpu;
    dma_addr_t			screen_dma;
    
    u16 *			palette_cpu;
    dma_addr_t			palette_dma; 
    u_int			palette_size;

    u_int			cmap_inverse:1,
    				cmap_static:1,
				unused:30;
	struct s3c2410fb_lcd_reg reg;
#ifdef CONFIG_PM
	struct pm_dev	*pm;
#endif
};

#define S3C2410_NAME		"S3C2410"

#define MIN_XRES	64
#define MIN_YRES	64
