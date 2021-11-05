#!/usr/bin/env python
from waflib.extras import autowaf as autowaf
import re

# Mandatory variables
top = '.'
out = 'build'

def options(opt):
	opt.load('compiler_c')
	opt.load('lv2')
	autowaf.set_options(opt)

def configure(conf):
	conf.load('compiler_c', cache=True)
	conf.load('lv2', cache=True)
	conf.load('autowaf', cache=True)

	conf.check_pkg('lv2', uselib_store='LV2')

	conf.check(features='c cshlib', lib='m', uselib_store='M', mandatory=False)

def build(bld):
	bundle = 'delta-limiter.lv2'

	# Build manifest.ttl by substitution (for portable lib extension)
	bld(features		= 'subst',
		source		= 'manifest.ttl.in',
		target		= 'lv2/%s/%s' % (bundle, 'manifest.ttl'),
		install_path	= '${LV2DIR}/%s' % bundle,
		LIB_EXT		= bld.env.LV2_LIB_EXT)

	# Copy other data files to build bundle
	for i in ['delta-limiter.ttl']:
		bld(features	= 'subst',
		is_copy		= True,
 		source		= i,
		target		= 'lv2/%s/%s' % (bundle, i),
		install_path	= '${LV2DIR}/%s' % bundle)

	# Build plugin library
	obj = bld(features	= 'c cshlib lv2lib',
		source		= 'delta-limiter.c',
		name		= 'delta-limiter',
		target		= 'lv2/%s/delta-limiter' % bundle,
		install_path	= '${LV2DIR}/%s' % bundle,
		uselib		= 'M LV2')
