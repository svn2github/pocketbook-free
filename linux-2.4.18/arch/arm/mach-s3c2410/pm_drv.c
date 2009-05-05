#include <linux/config.h>
#include <linux/pm.h>
#include <linux/module.h>

#include <asm/hardware.h>
/* Debugging macros */
#undef DEBUG_PMDRV
#ifdef DEBUG_PMDRV
#define DPRINTK(args...)	printk(##args)
#else
#define DPRINTK(args...)
#endif


extern int pm_do_suspend(void);

/* kernel/pm.c */
extern int pm_send_all_type(pm_dev_t type, pm_request_t rqst, void *data);

int 
pm_sys_suspend(void)
{
	int ret;

	DPRINTK("In "__FUNCTION__"\n");
	
	event_notify(SYSTEM_SLEEP);
#if 0
	ret = pm_send_all(PM_SUSPEND, (void *)2);
	if (ret) {
		pm_send_all(PM_RESUME, (void *)0);
		event_notify(SYSTEM_WAKEUP);
		return ret;
	}
#else
	ret = pm_send_all_type(PM_DEBUG_DEV, PM_SUSPEND, (void *)2);
	if (ret) {
		pm_send_all_type(PM_DEBUG_DEV, PM_RESUME, (void *)0);
		event_notify(SYSTEM_WAKEUP);
		return ret;
	}
#endif

	ret = pm_do_suspend();
	//pm_access(pm_dev);
	
#if 0
	ret = pm_send_all_type(PM_USER_DEV, PM_RESUME, (void *)0);
#else
	ret = pm_send_all_type(PM_DEBUG_DEV, PM_RESUME, (void *)0);
	if (ret) {
		printk("Warning. Somewrong while wakeup the system");
	}
#endif

	event_notify(SYSTEM_WAKEUP);

	//run_sbin_pm_helper(PM_RESUME);

	return ret;
}

int
pm_user_suspend(void)
{
	DPRINTK("In "__FUNCTION__"\n");
	//ret = pm_send_all_type(PM_USER_DEV, PM_SUSPEND, (void *)2);
	//ret = pm_send_target(PM_SYS_DEV, PM_SYS_PCMCIA, PM_SUSPEND, (void *)2);
	//
	pm_sys_suspend();

	//run_sbin_pm_helper(PM_SUSPEND);

	return 0;
}
EXPORT_SYMBOL(pm_user_suspend);

static char pm_helper_path[128] = "/usr/sbin/pm_helper";
extern int call_usermodehelper(char *path, char **argv, char **envp);

#if 0 // not yet ?
static void
run_sbin_pm_helper(pm_request_t req)
{
	int i;
	char *argv[3], *envp[8];

	if (!pm_helper_path[0])
		return;

	if (req != PM_SUSPEND && req != PM_RESUME)
		return;

	i = 0;
	argv[i++] = pm_helper_path;
	argv[i++] = (req == PM_RESUME ? "resume" : "suspend");
	argv[i] = 0;

#if 0
	DPRITNK(__FUNCTION__":%d pm_helper_path=%s\n", __LINE__,
		pm_helper_path);
#endif
	i = 0;
	/* minimal command environment */
	envp[i++] = "HOME=/";
	envp[i++] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
	envp[i] = 0;

	/* other stuff we want to pass to /sbin/pm_helper */
	call_usermodehelper(argv[0], argv, envp);
}
#endif /* not yet ? */
