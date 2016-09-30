#!/system/bin/sh
# a wrapper script to run dnsproxy2 service
#
# figure out the DNS server IP address
#
# from the following sources:
#   1) property persist.net.dns.server
#   2) environment variable DNS_SERVER_IP
#   3) Google public DNS 
# 
# the selected DNS IP reused upon reboot
#
SETP_CMD=/system/bin/setprop
GETP_CMD=/system/bin/getprop
DNS_PROP=persist.net.dns.server

_dns_ip=`$GETP_CMD $DNS_PROP`
if [ -z "$_dns_ip" ]; then
   if [ -n "$DNS_SERVER_IP" ]; then
      _dns_ip=$DNS_SERVER_IP
   else
      # fall back to Google public DNS
      _dns_ip=8.8.8.8
   fi

   $SETP_CMD $DNS_PROP $_dns_ip
fi

exec /system/bin/dnsproxy2 -w $_dns_ip
   

