#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#define MAX_RAMLOG_MSG_LEN 128

#define PROC_NAME "ramlog"

static char *output = "full";
static int order = 10;

module_param(output, charp, 0);
MODULE_PARM_DESC(output, "Output format: raw, time, full (defaul)");
module_param(order, int, 0);
MODULE_PARM_DESC(order, "Order size of the log (default 10)");

MODULE_LICENSE("GPL");

typedef struct {
	char buf[MAX_RAMLOG_MSG_LEN];
} ramlog_entry_t;

typedef enum { FMT_RAW, FMT_TIME, FMT_FULL } ramlog_fmt_t;

typedef struct {
	atomic_t lock; /* Lock to avoid log mutation when it is being printed */
	int page_order;
	struct page *page;

	ramlog_fmt_t format;

	/* Circular array. Invariants: */
#define IS_EMPTY(l) (atomic_read(&(l)->head) == (l)->mask)
#define MAKE_EMPTY(l) ({ atomic_set(&(l)->head, (l)->mask); (l)->is_full = 0; })
	int mask;
	int is_full;
	atomic_t head;
	ramlog_entry_t *entries;
} ramlog_t;

static ramlog_t *log;

static inline ramlog_entry_t *allocate_entry(ramlog_t *l)
{
	int head = atomic_inc_return(&l->head);
	int circular_head = head & l->mask;

	if (head != circular_head) // Try doing a modulo operation
		atomic_cmpxchg(&l->head, head, circular_head);
	if (circular_head == l->mask) // If the queue is fullfilled -- mark it
		l->is_full = 1;

	return &l->entries[circular_head];
}

int __ramlog(const char *fmt, ...)
{
	va_list args;
	ramlog_entry_t *e;
	ramlog_t *l = log;

	if (atomic_read(&l->lock) > 0) /* List is being print */
	    return 0;

	e = allocate_entry(l);
	va_start(args, fmt);
	vsnprintf(e->buf, sizeof(e->buf), fmt, args);
	va_end(args);

	return 0;
}
EXPORT_SYMBOL(__ramlog);

static ramlog_t *ramlog_alloc(int order, ramlog_fmt_t format)
{
	ramlog_t *l;
	int page_order;

	l = kmalloc(sizeof(ramlog_t), GFP_KERNEL);
	if (!l)
		return NULL;

	{ /* Calculate required page order */
		int a_size;
		int entries_size = (1 << order) * sizeof(ramlog_entry_t);

		for (page_order = 0, a_size = PAGE_SIZE; a_size < entries_size;
			page_order++, a_size <<= 1);
	}

	atomic_set(&l->lock, 0);
	l->page_order = page_order;
	l->page = alloc_pages(GFP_KERNEL, l->page_order);

	l->format = format;

	l->mask = (1 << order) - 1;
	MAKE_EMPTY(l);
	l->entries = page_address(l->page);

	{ /* Empty all log entries */
	    int i;

	    for (i = 0; i <= l->mask; i++)
		l->entries[i].buf[0] = 0;
	}

	printk(KERN_ERR "  ## %s():%d: log = { head = %d, full = %d, lock = %d\n",
		__FUNCTION__, __LINE__,
		atomic_read(&l->head), l->is_full, atomic_read(&l->lock));

	return l;
}

static void ramlog_free(ramlog_t *l)
{
	__free_pages(l->page, l->page_order);
	kfree(l);
}

static void *ramlog_seq_start(struct seq_file *s, loff_t *pos)
{
	static int idx = 0;
	ramlog_t *l = log;

	printk(KERN_ERR "  ## %s():%d: log = { head = %d, full = %d, lock = %d\n",
		__FUNCTION__, __LINE__,
		atomic_read(&l->head), l->is_full, atomic_read(&l->lock));

	/* Just finished an iteration -- empty and release the log*/
	if (*pos)
	{
		MAKE_EMPTY(l);
		atomic_dec_if_positive(&l->lock);
		return NULL;
	}

	/* Starting a new iteration */
	atomic_inc(&l->lock);

	/* The log is not full: iterate from 0 to head-1 */
	if (!l->is_full)
	{
	    if (IS_EMPTY(l))
	    {
		atomic_dec_if_positive(&l->lock);
		return NULL;
	    }

	    idx = 0;
	    return &idx;
	}

	/* The log is full: iterate from head+1 to head */
	idx = (atomic_read(&l->head) + 1) & l->mask;
	return &idx;
}

static void *ramlog_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	int *idx = (int *)v;
	ramlog_t *l = log;

	printk(KERN_ERR "  ## %s():%d:  pos = %lld, idx = %2d\n",
		__FUNCTION__, __LINE__, *pos, *idx);

	*idx = (*idx + 1) & l->mask;
	(*pos)++;

	return l->entries[*idx].buf[0] ? v : NULL;
}

static void ramlog_seq_stop(struct seq_file *s, void *v)
{
	/* nothing to do, we use a static value in start() */
	printk(KERN_ERR "  ## %s():%d:           idx = %2d\n",
		__FUNCTION__, __LINE__, v ? *((int *)v) : 0);
}

static int ramlog_seq_show(struct seq_file *s, void *v)
{
	int *idx = (int *) v;
	ramlog_t *l = log;

	printk(KERN_ERR "  ## %s():%d:           idx = %2d\n",
		__FUNCTION__, __LINE__, *idx);

	switch (l->format)
	{
	case FMT_RAW:
		seq_printf(s, "%s", l->entries[*idx].buf);
		break;
	case FMT_TIME:
		seq_printf(s, "[%8d] %s", 0, l->entries[*idx].buf);
		break;
	case FMT_FULL:
		seq_printf(s, "[%8d] %s():%d: %s", 0, "", 0,
			l->entries[*idx].buf);
		break;
	}

	l->entries[*idx].buf[0] = 0;

	return 0;
}

static struct seq_operations ramlog_seq_ops = {
	.start = ramlog_seq_start,
	.next  = ramlog_seq_next,
	.stop  = ramlog_seq_stop,
	.show  = ramlog_seq_show,
};

static ssize_t ramlog_proc_write(struct file *file, const char __user *buf,
	size_t len, loff_t *off)
{
	char msg[MAX_RAMLOG_MSG_LEN] = {};

	if (len >= MAX_RAMLOG_MSG_LEN)
		len = MAX_RAMLOG_MSG_LEN - 1;
	if (copy_from_user(msg, buf, len))
		return -EFAULT;

	__ramlog("%s\n", msg);

	return len;
}

static int ramlog_proc_open(struct inode *inode, struct file *file)
{
	if (atomic_read(&log->lock) > 0)
		return -EBUSY;

	return seq_open(file, &ramlog_seq_ops);
}

static struct file_operations ramlog_proc_fops = {
	.owner   = THIS_MODULE,
	.open    = ramlog_proc_open,
	.write   = ramlog_proc_write,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

static ramlog_fmt_t parse_format_param(void)
{
	if (!strcmp(output, "raw"))
		return FMT_RAW;
	else if (!strcmp(output, "time"))
		return FMT_TIME;
	else
		return FMT_FULL;
}

static int ramlog_init(void)
{
	struct proc_dir_entry *entry;

	printk(KERN_INFO "ramlog module being loaded.\n");

	if (!(log = ramlog_alloc(order, parse_format_param())))
	{
		printk(KERN_ERR "ramlog: cannot allocate memory.\n");
		return -1;
	}

	if (!(entry = create_proc_entry(PROC_NAME, 0666, NULL)))
	{
		printk(KERN_ERR "ramlog: cannot create procfs entry.\n");
		ramlog_free(log);
		return -1;
	}

	entry->proc_fops = &ramlog_proc_fops;

	return 0;
}

static void ramlog_uninit(void)
{
	printk(KERN_INFO "ramlog module being unloaded.\n");
	remove_proc_entry(PROC_NAME, NULL);
	ramlog_free(log);
}

module_init(ramlog_init);
module_exit(ramlog_uninit);
