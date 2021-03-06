# Module 'statcache'
#
# Maintain a cache of file stats.
# There are functions to reset the cache or to selectively remove items.

import os
from stat import *

# The cache.
# Keys are pathnames, values are `os.stat' outcomes.
#
cache = {}


# Stat a file, possibly out of the cache.
#
def stat(path):
	if cache.has_key(path):
		return cache[path]
	cache[path] = ret = os.stat(path)
	return ret


# Reset the cache completely.
#
def reset():
	global cache
	cache = {}


# Remove a given item from the cache, if it exists.
#
def forget(path):
	if cache.has_key(path):
		del cache[path]


# Remove all pathnames with a given prefix.
#
def forget_prefix(prefix):
	n = len(prefix)
	for path in cache.keys():
		if path[:n] == prefix:
			del cache[path]


# Forget about a directory and all entries in it, but not about
# entries in subdirectories.
#
def forget_dir(prefix):
	if prefix[-1:] == '/' and prefix <> '/':
		prefix = prefix[:-1]
	forget(prefix)
	if prefix[-1:] <> '/':
		prefix = prefix + '/'
	n = len(prefix)
	for path in cache.keys():
		if path[:n] == prefix:
			rest = path[n:]
			if rest[-1:] == '/': rest = rest[:-1]
			if '/' not in rest:
				del cache[path]


# Remove all pathnames except with a given prefix.
# Normally used with prefix = '/' after a chdir().
#
def forget_except_prefix(prefix):
	n = len(prefix)
	for path in cache.keys():
		if path[:n] <> prefix:
			del cache[path]


# Check for directory.
#
def isdir(path):
	try:
		st = stat(path)
	except os.error:
		return 0
	return S_ISDIR(st[ST_MODE])
