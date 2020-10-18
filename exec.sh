#!/bin/bash

pushd container
	go build 
	go install
popd
cp -r container ~/Go/src/github.com/glin/container
go build .
./glin run -ti /bin/sh
