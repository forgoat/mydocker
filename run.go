package main

import (
	log "github.com/Sirupsen/logrus"
	"glin/container"
	"os"
)

func Run(tty bool, cmd string) {
	parent := container.NewParentProcess(tty, cmd)
	if err := parent.Start(); err != nil {
		log.Error(err)
	}
	if err := parent.Wait(); err != nil {
		log.Fatal(err)
	}
	os.Exit(-1)
}
