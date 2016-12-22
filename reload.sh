#!/bin/bash

APPNAME=ppn
MANIFEST=manifest.json
PROFILE=DefaultProfile


PYTEST_APP=Echo

CT=cocaine-tool

function reload() {
	$CT app stop -n $1
	$CT app remove -n $1
	$CT app upload -n $1 $4 --manifest $2
	$CT app start -n $1 -r $3
}

reload $APPNAME $MANIFEST $PROFILE .

PY_APP_NAME=Echo
PY_APP_MANIFEST='py-echo-manifest.json'

reload $PY_APP_NAME $PY_APP_MANIFEST $PROFILE ..

