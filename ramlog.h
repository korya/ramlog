#ifndef __RAMLOG_H__
#define __RAMLOG_H__

#define ramlog(fmt...) __ramlog(fmt)
int __ramlog(const char *fmt, ...);

#endif
