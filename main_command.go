package main

import (
	"fmt"
	log "github.com/Sirupsen/logrus"
	"github.com/urfave/cli"
	"glin/container"
)

var runCommand = cli.Command{
	Name:  "run",
	Usage: "medocker run -ti [command]",
	Flags: []cli.Flag{
		cli.BoolFlag{
			Name:  "ti",
			Usage: "enable tty",
		},
	},
	Action: func(ctx *cli.Context) error {
		if len(ctx.Args()) < 1 {
			return fmt.Errorf("Missing container command")
		}
		cmd := ctx.Args().Get(0)
		tty := ctx.Bool("ti")
		Run(tty, cmd)
		return nil
	},
}

var initCommand = cli.Command{
	Name:  "init",
	Usage: "Do not call it outside",
	Action: func(ctx *cli.Context) error {
		log.Infof("init come on")
		cmd := ctx.Args().Get(0)
		log.Infof("command initcommand %s", cmd)
		err := container.RunContainerInitProcess(cmd, nil)
		if err != nil {
			fmt.Println("Error ", err)
		}
		return err
	},
}
