#!/bin/bash

ps auxw | grep mangband | grep -v grep > /dev/null

if [ $? != 0 ]
then
       ./mangband
fi
