# A class that makes each part of a multipart message "feel" like an
# ordinary file, as long as you use fp.readline().  Allows recursive
# use, for nested multipart messages.  Probably best used together
# with module mimetools.
#
# Suggested use:
#
# real_fp = open(...)
# fp = MultiFile(real_fp)
#
# "read some lines from fp"
# fp.push(separator)
# while 1:
#	"read lines from fp until it returns an empty string" (A)
#	if not fp.next(): break
# fp.pop()
# "read remaining lines from fp until it returns an empty string"
#
# The latter sequence may be used recursively at (A).
# It is also allowed to use multiple push()...pop() sequences.
#
# If seekable is given as 0, the class code will not do the bookeeping
# it normally attempts in order to make seeks relative to the beginning of the
# current file part.  This may be useful when using MultiFile with a non-
# seekable stream object.

import sys
import string

Error = 'multifile.Error'

class MultiFile:
	#
	seekable = 0
	#
	def __init__(self, fp, seekable=1):
		self.fp = fp
		self.stack = [] # Grows down
		self.level = 0
		self.last = 0
		if seekable:
			self.seekable = 1
			self.start = self.fp.tell()
			self.posstack = [] # Grows down
	#
	def tell(self):
		if self.level > 0:
			return self.lastpos
		return self.fp.tell() - self.start
	#
	def seek(self, pos, whence=0):
		here = self.tell()
		if whence:
			if whence == 1:
				pos = pos + here
			elif whence == 2:
				if self.level > 0:
					pos = pos + self.lastpos
				else:
					raise Error, "can't use whence=2 yet"
		if not 0 <= pos <= here or \
				self.level > 0 and pos > self.lastpos:
			raise Error, 'bad MultiFile.seek() call'
		self.fp.seek(pos + self.start)
		self.level = 0
		self.last = 0
	#
	def readline(self):
		if self.level > 0:
			return ''
		line = self.fp.readline()
		# Real EOF?
		if not line:
			self.level = len(self.stack)
			self.last = (self.level > 0)
			if self.last:
				raise Error, 'sudden EOF in MultiFile.readline()'
			return ''
		assert self.level == 0
		# Fast check to see if this is just data
		if self.is_data(line):
			return line
		else:
			# Ignore trailing whitespace on marker lines 
			k = len(line) - 1;
			while line[k] in string.whitespace:
				k = k - 1
			marker = line[:k+1]
		# No?  OK, try to match a boundary.
		# Return the line (unstripped) if we don't.
		for i in range(len(self.stack)):
			sep = self.stack[i]
			if marker == self.section_divider(sep):
				self.last = 0
				break
			elif marker == self.end_marker(sep):
				self.last = 1
				break
		else:
			return line
		# We only get here if we see a section divider or EOM line
		if self.seekable:
			self.lastpos = self.tell() - len(line)
		self.level = i+1
		if self.level > 1:
			raise Error,'Missing endmarker in MultiFile.readline()'
		return ''
	#
	def readlines(self):
		list = []
		while 1:
			line = self.readline()
			if not line: break
			list.append(line)
		return list
	#
	def read(self): # Note: no size argument -- read until EOF only!
		return string.joinfields(self.readlines(), '')
	#
	def next(self):
		while self.readline(): pass
		if self.level > 1 or self.last:
			return 0
		self.level = 0
		self.last = 0
		if self.seekable:
			self.start = self.fp.tell()
		return 1
	#
	def push(self, sep):
		if self.level > 0:
			raise Error, 'bad MultiFile.push() call'
		self.stack.insert(0, sep)
		if self.seekable:
			self.posstack.insert(0, self.start)
			self.start = self.fp.tell()
	#
	def pop(self):
		if self.stack == []:
			raise Error, 'bad MultiFile.pop() call'
		if self.level <= 1:
			self.last = 0
		else:
			abslastpos = self.lastpos + self.start
		self.level = max(0, self.level - 1)
		del self.stack[0]
		if self.seekable:
			self.start = self.posstack[0]
			del self.posstack[0]
			if self.level > 0:
				self.lastpos = abslastpos - self.start
	#
	def is_data(self, line):
		return line[:2] <> '--'
	#
	def section_divider(self, str):
		return "--" + str
	#
	def end_marker(self, str):
		return "--" + str + "--"
