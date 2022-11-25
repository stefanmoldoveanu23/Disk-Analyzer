#!/bin/bash

case "$1" in
	create)
	sudo cp manager_da.service /etc/systemd/system/manager_da.service
	sudo systemctl daemon-reload
	sudo systemctl start manager_da
	sudo systemctl enable manager_da
	;;
	delete)
	sudo systemctl stop manager_da
	sudo systemctl disable manager_da
	sudo rm /etc/systemd/system/manager_da.service
	sudo systemctl daemon-reload
	sudo systemctl reset-failed
	;;
	*)
	echo "Wrong argument. Need create or delete."
	;;
esac
