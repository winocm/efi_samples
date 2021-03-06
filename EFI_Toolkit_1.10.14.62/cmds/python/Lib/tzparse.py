# Parse a timezone specification.
# XXX Unfinished.
# XXX Only the typical form "XXXhhYYY;ddd/hh,ddd/hh" is currently supported.

tzpat = ('^([A-Z][A-Z][A-Z])([-+]?[0-9]+)([A-Z][A-Z][A-Z]);'
	  '([0-9]+)/([0-9]+),([0-9]+)/([0-9]+)$')

tzprog = None

def tzparse(tzstr):
	global tzprog
	if tzprog == None:
		import re
		tzprog = re.compile(tzpat)
	match = tzprog.match(tzstr)
	if not match:
		raise ValueError, 'not the TZ syntax I understand'
	subs = []
	for i in range(1, 8):
		subs.append(match.group(i))
	for i in (1, 3, 4, 5, 6):
		subs[i] = eval(subs[i])
	[tzname, delta, dstname, daystart, hourstart, dayend, hourend] = subs
	return (tzname, delta, dstname, daystart, hourstart, dayend, hourend)

def tzlocaltime(secs, params):
	import time
	(tzname, delta, dstname, daystart, hourstart, dayend, hourend) = params
	year, month, days, hours, mins, secs, yday, wday, isdst = \
		time.gmtime(secs - delta*3600)
	if (daystart, hourstart) <= (yday+1, hours) < (dayend, hourend):
		tzname = dstname
		hours = hours + 1
	return year, month, days, hours, mins, secs, yday, wday, tzname

def tzset():
	global tzparams, timezone, altzone, daylight, tzname
	import os
	tzstr = os.environ['TZ']
	tzparams = tzparse(tzstr)
	timezone = tzparams[1] * 3600
	altzone = timezone - 3600
	daylight = 1
	tzname = tzparams[0], tzparams[2]

def isdst(secs):
	import time
	(tzname, delta, dstname, daystart, hourstart, dayend, hourend) = \
		tzparams
	year, month, days, hours, mins, secs, yday, wday, isdst = \
		time.gmtime(secs - delta*3600)
	return (daystart, hourstart) <= (yday+1, hours) < (dayend, hourend)

tzset()

def localtime(secs):
	return tzlocaltime(secs, tzparams)

def test():
	from time import asctime, gmtime
	import time, sys
	now = time.time()
	x = localtime(now)
	tm = x[:-1] + (0,)
	print 'now =', now, '=', asctime(tm), x[-1]
	now = now - now % (24*3600)
	if sys.argv[1:]: now = now + eval(sys.argv[1])
	x = gmtime(now)
	tm = x[:-1] + (0,)
	print 'gmtime =', now, '=', asctime(tm), 'yday =', x[-2]
	jan1 = now - x[-2]*24*3600
	x = localtime(jan1)
	tm = x[:-1] + (0,)
	print 'jan1 =', jan1, '=', asctime(tm), x[-1]
	for d in range(85, 95) + range(265, 275):
		t = jan1 + d*24*3600
		x = localtime(t)
		tm = x[:-1] + (0,)
		print 'd =', d, 't =', t, '=', asctime(tm), x[-1]
