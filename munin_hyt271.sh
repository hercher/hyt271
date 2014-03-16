#!/bin/bash

HYT_READ="/usr/local/bin/hyt271"

if [ "$1" = "autoconf" ]
then
        echo "yes"
        exit 0
fi

if [ "$1" = "config" ]
then
        echo 'graph_title HYT Humidity and Temperature'
        echo 'graph_args --base 1000 -l 0'
        echo 'graph_vlabel Humidity/Temperature'
        echo 'graph_scale yes'
        echo 'graph_category other'
        echo 'hyt_hum.label Humidity'
        echo 'hyt_temp.label Temperature'
        echo 'graph_info HYT Humidity and Temperature'
        echo 'hyt_hum.info Humidity'
        echo 'hyt_temp.info Temperature'
        exit 0
fi

VAL=$($HYT_READ)

if [ $? -ne 0 ]
then
	echo "hyt_temp.value U"
	echo "hyt_hum.value U"
	exit 0
fi

echo $VAL | awk '{ print "hyt_temp.value "$1; print "hyt_hum.value "$2; }'
