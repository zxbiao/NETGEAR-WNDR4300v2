#!/bin/sh
# (C) 2008 openwrt.org

. /etc/functions.sh
STATUS=$1
hnat_enable() {
	echo "***************enable hnat******************" > /dev/console
	echo 1 > /proc/qca_switch/nf_athrs17_hnat
	
	ssdk_sh misc ptarpack set 1 disable
	ssdk_sh misc ptarpack set 2 disable
	ssdk_sh misc ptarpack set 3 disable
	ssdk_sh misc ptarpack set 4 disable
	ssdk_sh misc ptarpack set 5 disable
	ssdk_sh misc ptarpack set 6 disable

	ssdk_sh acl status set enable
	ssdk_sh acl list create 1 1
	ssdk_sh acl rule add 1 0 1 mac n n y 0x0806 0xffff n n n n n n n n n n n n n n y n n n y n n n n n n 0 0 n n n n n n n n n n n n n
	ssdk_sh acl list bind 1 0 0 1
	ssdk_sh acl list bind 1 0 0 2
	ssdk_sh acl list bind 1 0 0 3
	ssdk_sh acl list bind 1 0 0 4
	ssdk_sh acl list bind 1 0 0 5
	ssdk_sh acl list bind 1 0 0 6

	ssdk_sh ip ptarpsrcguard set 1 no_guard
	ssdk_sh ip ptarpsrcguard set 2 no_guard
	ssdk_sh ip ptarpsrcguard set 3 no_guard
	ssdk_sh ip ptarpsrcguard set 4 no_guard
	ssdk_sh ip ptarpsrcguard set 5 no_guard
	ssdk_sh ip ptarpsrcguard set 6 no_guard	
}

hnat_disable() {
	echo "***************disable hnat******************" > /dev/console
	echo 0 > /proc/qca_switch/nf_athrs17_hnat
}

hnat_restart() {
	echo "****************restart hnat*****************" > /dev/console
	echo 0 > /proc/qca_switch/nf_athrs17_hnat
	sleep 5
	echo 1 > /proc/qca_switch/nf_athrs17_hnat
	sleep 1
}

hnat_help() {
	echo "********usage: hnat.sh enable/disable******************"
}

case $STATUS in
    enable)
        hnat_enable
        ;;
    disable)
        hnat_disable
        ;;
    restart)
        hnat_restart
	;;
    *)
        hnat_help
	;;
esac
