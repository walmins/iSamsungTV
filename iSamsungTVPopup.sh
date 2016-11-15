#!/bin/bash
iSamsungTV $1 -SMS "" "" "" "" "" "" $2
SLEEP 0.3
iSamsungTV $1 -KEY KEY_ENTER
SLEEP 3
iSamsungTV $1 -KEY KEY_ENTER