#!/bin/bash
iSamsungTV $1 -SMS "" "" "" "" "" "" $2
sleep 0.3
iSamsungTV $1 -KEY KEY_ENTER
sleep 3
iSamsungTV $1 -KEY KEY_ENTER