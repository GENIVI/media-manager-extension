#!/bin/sh
JS=$1
COUT=$2

if [ ! $JS -o ! $COUT ] ; then
  echo "Usage $0 <js api file> <output c header file>"
  exit 1
fi

if [ ! -d "`dirname $COUT`" ] ; then
    mkdir -p "`dirname $COUT`"
fi

echo -n "static const char* kSource_echo_api = \"" > $COUT

cat $JS | awk -F\n '{print "" $_ "\\"}' | \
  tr -d $'\r' >> $COUT

echo "\";" >> $COUT
