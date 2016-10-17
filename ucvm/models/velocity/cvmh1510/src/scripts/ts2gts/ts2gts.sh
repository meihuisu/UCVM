#!/bin/bash

awk -f ts2gts.awk $1 | /usr/share/gts/examples/cleanup 1
