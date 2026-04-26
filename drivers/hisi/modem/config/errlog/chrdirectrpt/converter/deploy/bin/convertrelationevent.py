#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2010-2019. All rights reserved.

import re

class Event:
	event = ''
	subevents = []
	
	def __init__(self, event, subevent):
		self.event = event
		self.subevents = []
		self.subevents.append(subevent)
		
	def __str__(self):
		return self.event + ":" + ",".join(self.subevents)

result = []

def AddSubItem(event, subevent):
	for r in result:
		if r.event == event:
			r.subevents.append(subevent)
			return
	ev = Event(event, subevent)
	result.append(ev)

def Output(path):
	with open(path, 'w') as file:
		s = ''
		for r in result:
			for f in r.subevents:
				s = '%s%s%s%s%s' %(s, r.event, '    ', f, '\n')
		file.write(s)

	
def ConvertAbsoluteEvent(path):
	with open(path, 'r') as file:
		data = file.read()
		m = '.*int[\s]+g_[^\[]*\[[^\]]*\][\s]*\[[^\]]*\][\s]*=[\s]*\{[\s]*([^;]*)\}[\s]*;'
		text = re.findall(m, data)
	for r in text:
		m = '\{[\s]*([^\s,]*)[\s]*,[\s]*([^\s,]*)[\s]*\}'
		items = re.findall(m, r)
		for i in items:
			AddSubItem(i[0][:i[0].index('_EVENTID')], i[1][:i[1].index('_ALARMID')])

ConvertAbsoluteEvent('../../../src/ChrAbsoluteEvent_cfg.c')
Output('./ChrEvent.c')