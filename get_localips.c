#include <stdio.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>

int interface_get_local_ips()
{
  struct sockaddr_in* sin = NULL;
  struct ifaddrs* ifa = NULL, * ifList;

  if (getifaddrs(&ifList) < 0)
  {
    return -1;
  }

  for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next) {
    /* no ip address from interface that is down */
    if ((ifa->ifa_flags & IFF_UP) == 0) {
      sin = (struct sockaddr_in*)ifa->ifa_addr;
      printf(">>> interface: %s %s is down\n", ifa->ifa_name, inet_ntoa(sin->sin_addr));
      continue;
    }

    /* no ip address from interface that isn't running */
    if ((ifa->ifa_flags & IFF_RUNNING) == 0) {
      sin = (struct sockaddr_in*)ifa->ifa_addr;
      printf(">>> interface: %s %s isn't running\n", ifa->ifa_name, inet_ntoa(sin->sin_addr));
      continue;
    }

    if (ifa->ifa_addr == NULL)
      continue;

    if (ifa->ifa_addr->sa_family != AF_INET && ifa->ifa_addr->sa_family != AF_INET6) {
      printf(">>> interfase: %s not IPV4 or IPV6 , %d\n", ifa->ifa_name, ifa->ifa_addr->sa_family);
      continue;
    }

    if ((ifa->ifa_flags & IFF_LOOPBACK) == IFF_LOOPBACK) {
      sin = (struct sockaddr_in*)ifa->ifa_addr;
      printf(">>> loopback address: %s\n", inet_ntoa(sin->sin_addr));
      continue;
    }

    if (ifa->ifa_addr->sa_family == AF_INET)
    {
      sin = (struct sockaddr_in*)ifa->ifa_addr;
      printf(">>> interface: %s %s\n", ifa->ifa_name, inet_ntoa(sin->sin_addr));
    }
  }

  freeifaddrs(ifList);
  return 0;
}

#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
void get_ip_for_interface(char* interface_name)
{
  struct ifreq ifr;
  union {
    struct sockaddr* addr;
    struct sockaddr_in* in;
  } sa;
  int sockfd;

  ifr.ifr_addr.sa_family = AF_INET;
  memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
  strncpy(ifr.ifr_name, interface_name, sizeof(ifr.ifr_name));

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
    printf("Error : Cannot open socket to retrieve interface list");
    return;
  }

  if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
    printf("Error : Unable to get IP information for interface %s",
      interface_name);
    close(sockfd);
    return;
  }

  close(sockfd);
  sa.addr = &ifr.ifr_addr;
  printf("Address for %s: %s", interface_name, inet_ntoa(sa.in->sin_addr));
}
#include <stdio.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
 
int get_localip(const char * eth_name, char *local_ip_addr)
{
	int ret = -1;
    register int fd, intrface;
    struct ifreq ifr[32];
    struct ifconf ifc;
 
    if (local_ip_addr == NULL || eth_name == NULL)
	{
        return ret;
	}
	if ((fd=socket(AF_INET, SOCK_DGRAM, 0)) > 0)
	{
		ifc.ifc_len = sizeof ifr;
		ifc.ifc_buf = (caddr_t)ifr;
		if (!ioctl(fd, SIOCGIFCONF, (char*)&ifc)) //获取所有接口信息
		{
			intrface = ifc.ifc_len / sizeof(struct ifreq);
			while (intrface-- > 0)
			{
				//Get IP Address
				if (!(ioctl(fd, SIOCGIFADDR, (char*)&ifr[intrface])))
				{
					if(strcmp(eth_name, ifr[intrface].ifr_name) == 0)
					{
						ret = 0;
						sprintf(local_ip_addr, "%s", inet_ntoa(((struct sockaddr_in*)(&ifr[intrface].ifr_addr))->sin_addr));
						break;
					}
				}
			}
		}
	}
	if (fd > 0)
	{
		close(fd);
	}
    return ret;
}
 
int main(int argc, const char **argv)
{
	int ret;
	char local_ip[20] = {0};
 
	ret = get_localip("eth0", local_ip);
	if (ret == 0)
	{
		printf("local ip:%s\n", local_ip);
	}
	else
	{
		printf("get local ip failure\n");
	}
	return 0;
}
