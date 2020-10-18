package container

import (
	"fmt"
	log "github.com/Sirupsen/logrus"
	"os"
	"syscall"
)

func RunContainerInitProcess(cmd string, args []string) error {
	log.Infof("command init %s", cmd)
	/*	if err := syscall.Mount("", "/", "", syscall.MS_PRIVATE|syscall.MS_REC, ""); err != nil {
		fmt.Println("Error1 ", err)
	}*/
	defaultMountFlags := syscall.MS_NOEXEC | syscall.MS_NOSUID | syscall.MS_NODEV
	if err := syscall.Mount("proc", "/proc", "proc", uintptr(defaultMountFlags), ""); err != nil {
		fmt.Println("Error2 ", err)
	}
	argv := []string{cmd}

	if err := syscall.Exec(cmd, argv, os.Environ()); err != nil {
		fmt.Println("Error3 ", err)
		log.Errorf(err.Error())
	}
	fmt.Println("3")
	return nil
}
