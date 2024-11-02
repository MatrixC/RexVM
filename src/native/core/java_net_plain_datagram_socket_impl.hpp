#ifndef NATIVE_CORE_JAVA_NET_PLAIN_DATA_GRAM_SOCKET_HPP
#define NATIVE_CORE_JAVA_NET_PLAIN_DATA_GRAM_SOCKET_HPP
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <cstring>
#include "../../basic.hpp"
#include "../../frame.hpp"

namespace RexVM::Native::Core {

    //native void bind0(int lport, InetAddress laddr)
    void udpBind0(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto sockfd = socketGetFdFieldValue(self);
        const auto port = frame.getLocalI4(1);
        const auto laddr = frame.getLocalI4(2);
        
        struct sockaddr_in serAddr;
        bzero(&serAddr, sizeof(serAddr));
        serAddr.sin_family = AF_INET;
        serAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serAddr.sin_port = htons(port);
        // TODO 此处忽略的bind ip

        if (bind(sockfd, (struct sockaddr*)&serAddr, sizeof(serAddr)) < 0) {
            throwSocketException(frame,"bind error");
            return;
        }
    }


}

#endif