#!/bin/bash

export AL_USDMAYA_LOCATION=$1
export USD_LIBRARY_PATH=$2
export MAYA_PLUG_IN_PATH=$AL_USDMAYA_LOCATION/plugin:$MAYA_PLUG_IN_PATH
export LD_LIBRARY_PATH=$AL_USDMAYA_LOCATION/lib:$USD_LIBRARY_PATH:$LD_LIBRARY_PATH
export PYTHONPATH=$AL_USDMAYA_LOCATION/lib/python:$USD_LIBRARY_PATH/python:$3:$PYTHONPATH
export PXR_PLUGINPATH=$AL_USDMAYA_LOCATION/share/usd/plugins:$PXR_PLUGINPATH
export PATH=$MAYA_LOCATION/bin:$PATH

maya -batch -command "python(\"execfile(\\\"$3/testTranslators.py\\\")\")"

exit $?
