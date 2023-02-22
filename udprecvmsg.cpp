// first set options level IPPROTO_IP type IP_PKTINFO
// g_socket_set_option(_sock, IPPROTO_IP, IP_PKTINFO, 1, NULL);
static bool RecvMessage(int fd, struct sockaddr_in* from, struct sockaddr_in* to, char* buffer, int* length) {
  char cmbuf[100];
  struct iovec iov[1];
  iov[0].iov_base = buffer;
  iov[0].iov_len = PROXY_UDP_RECV_BUFFER;
  struct msghdr mh = {
      .msg_name = from,
      .msg_namelen = sizeof(struct sockaddr_in),
      .msg_iov = iov,
      .msg_iovlen = 1
      .msg_control = cmbuf,
      .msg_controllen = sizeof(cmbuf),
  };
  int n = recvmsg(fd, &mh, 0);
  if (n < 0) {
    printf("recvfrom err in udptalk!\n");
    return false;
  }
  else {
    cmbuf[n] = 0;
    printf("Receive:%dByte。\tThe Message Is:%s\n", n, buffer);
    *length = n;
  }

  struct cmsghdr* cmsg;
  for (cmsg = CMSG_FIRSTHDR(&mh); cmsg != NULL; cmsg = CMSG_NXTHDR(&mh, cmsg)) {
    // filter IP PKTINFO messae
    if (cmsg->cmsg_level != IPPROTO_IP ||
      cmsg->cmsg_type != IP_PKTINFO) {
      continue;
    }

    struct in_pktinfo* pi = CMSG_DATA(cmsg);
    char dst[100], ipi[100];
    //
    memcpy(&pi->ipi_spec_dst, &to->sin_addr)
    if ((inet_ntop(AF_INET, &(pi->ipi_spec_dst), dst, sizeof(dst)
    )) != NULL) {
      printf("IPto IPdst=%s\n", dst);
    }
    if ((inet_ntop(AF_INET, &(pi->ipi_addr), ipi, sizeof(ipi)
    )) != NULL) {
      printf("to ipi_addr=%s\n", ipi);
    }
  }
  char src[100];
  if (inet_ntop(AF_INET, &from->sin_addr, src, sizeof(src)) != NULL) {
    printf("from:%s\n", src);
  }
  return true;
}
