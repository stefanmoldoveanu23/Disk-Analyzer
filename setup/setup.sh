#!/bin/bash

case "$1" in
	create)
	echo -e "#ifndef PATHS_H\n#define PATHS_H\n\n#define ANALYSES_PATH \"$(realpath ../data/analyses)\"\n#define DIR_PATH \"$(realpath ../data)/\"\n\n#endif" > ../dstructs/paths.h

	echo -e "[Unit]\nDescription= Disk Analyzer manager startup and shutdown helper\n\n[Service]\nExecStart=$(realpath ../manager/manager)\nKillMode=process\nStandardOutput=journal+console\nTimeoutStopSec=10\nSendSIGKILL=no\n\n[Install]\nWantedBy=multi-user.target" > manager_da.service

	sudo cp manager_da.service /etc/systemd/system/manager_da.service
	sudo systemctl daemon-reload
	sudo systemctl start manager_da
	sudo systemctl enable manager_da

	sudo echo "alias da='sudo $(realpath ../client/client)'" >> ~/.bash_aliases
	. ~/.bashrc
	;;
	delete)
	sudo systemctl stop manager_da
	sudo systemctl disable manager_da
	sudo rm /etc/systemd/system/manager_da.service
	sudo systemctl daemon-reload
	sudo systemctl reset-failed

	sudo sed -i.bak '/alias da/d' ~/.bash_aliases
	. ~/.bashrc
	;;
	*)
	echo "Wrong argument. Need create or delete."
	;;
esac
