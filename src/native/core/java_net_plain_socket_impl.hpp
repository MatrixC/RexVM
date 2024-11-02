#ifndef NATIVE_CORE_JAVA_NET_PLAIN_SOCKET_HPP
#define NATIVE_CORE_JAVA_NET_PLAIN_SOCKET_HPP
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno>
#include "../../basic.hpp"
#include "../../frame.hpp"

namespace RexVM::Native::Core {

    i4 socketGetFdFieldValue(InstanceOop *obj) {
        const auto fdOop = CAST_INSTANCE_OOP(obj->getFieldValue("fd" "Ljava/io/FileDescriptor;").refVal);
        const auto fdVal = fdOop->getFieldValue("fd" "I").i4Val;
        return fdVal;
    }

    void socketSetFdFieldValue(InstanceOop *obj, i4 val) {
        const auto fdOop = CAST_INSTANCE_OOP(obj->getFieldValue("fd" "Ljava/io/FileDescriptor;").refVal);
        fdOop->setFieldValue("fd" "I", Slot(CAST_I4(val)));
    }

    cstring socketGetHostName(InstanceOop *inetAddressOop) {
        const auto holderOop = CAST_INSTANCE_OOP(inetAddressOop->getFieldValue("holder" "Ljava/net/InetAddress$InetAddressHolder;").refVal);
        const auto hostNameOop = CAST_INSTANCE_OOP(holderOop->getFieldValue("hostName" "Ljava/lang/String;").refVal);
        return VMStringHelper::getJavaString(CAST_INSTANCE_OOP(hostNameOop));
    }

    i4 socketGetAddress(InstanceOop *inetAddressOop) {
        const auto holderOop = CAST_INSTANCE_OOP(inetAddressOop->getFieldValue("holder" "Ljava/net/InetAddress$InetAddressHolder;").refVal);
        return holderOop->getFieldValue("address" "I").i4Val;
    }

    i4 socketGetFamily(InstanceOop *inetAddressOop) {
        const auto holderOop = CAST_INSTANCE_OOP(inetAddressOop->getFieldValue("holder" "Ljava/net/InetAddress$InetAddressHolder;").refVal);
        return holderOop->getFieldValue("family" "I").i4Val;
    }

    void socketSetAddressOrFamily(InstanceOop *inetAddressOop, i4 val, u1 type) {
        //type == 0:address
        //type == 1:family
        const auto holderOop = CAST_INSTANCE_OOP(inetAddressOop->getFieldValue("holder" "Ljava/net/InetAddress$InetAddressHolder;").refVal);
        if (type == 0) {
            holderOop->setFieldValue("address" "I", Slot(val));
        } else {
            holderOop->setFieldValue("family" "I", Slot(val));
        }
    }

    //static native void initProto();
    void socketInitProto(Frame &frame) {
    }

    //native void socketCreate(boolean isServer) throws IOException;
    void socketCreate(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        // const auto isServer = frame.getLocalBoolean(1); is true, no use

        const auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            const auto errorMsg = std::strerror(errno);
            throwIOException(frame, "create socket error: " + cstring{errorMsg});
            return;
        }
        socketSetFdFieldValue(self, sockfd);
    }

    //native void socketConnect(InetAddress address, int port, int timeout) throws IOException;
    void socketConnect(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto sockfd = socketGetFdFieldValue(self);
        const auto addressOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(addressOop)
        const auto port = frame.getLocalI4(2);
        const auto timeout = frame.getLocalI4(3);

        struct sockaddr_in servAddr;
        bzero(&servAddr, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = swap32(CAST_U4(socketGetAddress(addressOop)));
        servAddr.sin_port = swap16(CAST_U2(port));
        if (connect(sockfd, (sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
            const auto errorMsg = std::strerror(errno);
            throwIOException(frame, "connect error: " + cstring{errorMsg});
            return;
        }
    }

    //native void socketBind(InetAddress address, int port)
    void socketBind(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto addressOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(addressOop)
        const auto port = frame.getLocalI4(2);
        const auto sockfd = socketGetFdFieldValue(self);

        const auto hostName = socketGetHostName(addressOop);
        const auto family = socketGetFamily(addressOop);

        if (family == 1) {
            //IPV4
            struct sockaddr_in servAddr;
            bzero(&servAddr, sizeof(servAddr));

            servAddr.sin_family = AF_INET;
            servAddr.sin_addr.s_addr = inet_addr(hostName.c_str());
            servAddr.sin_port = swap16(CAST_U2(port));

            if (bind(sockfd, (sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
                const auto errorMsg = std::strerror(errno);
                throwIOException(frame, "bind address error: " + cstring{errorMsg});
                return;
            }
        } else {
            //IPV6
            throwIOException(frame, "not support ipv6");
            return;
        }
    }

    //native void socketListen(int count) throws IOException;
    void socketListen(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto count = frame.getLocalI4(1);
        const auto sockfd = socketGetFdFieldValue(self);

        if (listen(sockfd, count) < 0) {
            const auto errorMsg = std::strerror(errno);
            throwIOException(frame, "listen error: " + cstring{errorMsg});
            return;
        }
    }

    //native void socketAccept(SocketImpl s) throws IOException;
    void socketAccept(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto s = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(s)
        const auto sockfd = socketGetFdFieldValue(self);

        struct sockaddr_in clntAddr;
        socklen_t addrLen = sizeof(sockaddr_in);
        bzero(&clntAddr, sizeof(clntAddr));
        const auto clientSockfd = accept(sockfd, (sockaddr*)&clntAddr, &addrLen);
        if (clientSockfd == -1) {
            const auto errorMsg = std::strerror(errno);
            throwIOException(frame, "accept error: " + cstring{errorMsg});
            return;
        }
        socketSetFdFieldValue(s, clientSockfd);
        s->setFieldValue("port" "I", Slot(CAST_I4(ntohs(clntAddr.sin_port))));
        
        struct sockaddr_in servAddr;
        bzero(&servAddr, addrLen);
        if (getsockname(sockfd, (struct sockaddr*)&servAddr, &addrLen) == 0) {
            const auto localPort = ntohs(servAddr.sin_port);
            s->setFieldValue("localport" "I", Slot(CAST_I4(localPort)));
        }

        const auto inetV4Class = frame.getCurrentClassLoader()->getInstanceClass("java/net/Inet4Address");
        const auto inetOop = frame.mem.newInstance(inetV4Class);
        frame.addCreateRef(inetOop);
        const auto initMethod = inetV4Class->getMethod("<init>" "(Ljava/lang/String;I)V", false);
        const auto u4Address = swap32(CAST_U4(clntAddr.sin_addr.s_addr));
        frame.runMethodManual(*initMethod, {Slot(inetOop), Slot(nullptr), Slot(CAST_I4(u4Address))});
        s->setFieldValue("address" "Ljava/net/InetAddress;", Slot(inetOop));
    }

    //native void socketClose0(boolean useDeferredClose) throws IOException;
    void socketClose0(Frame &frame) {
        const auto self = frame.getThisInstance();
        const auto useDeferredClose = frame.getLocalBoolean(1);
        const auto sockfd = socketGetFdFieldValue(self);

        ::close(sockfd);
    }

    // native int socketAvailable() throws IOException;
    void socketAvailable(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto sockfd = socketGetFdFieldValue(self);
        
        i4 availableBytes;
        if (ioctl(sockfd, FIONREAD, &availableBytes) < 0) {
            const auto errorMsg = std::strerror(errno);
            throwIOException(frame, "ioctl error: " + cstring{errorMsg});
            return;
        }
        frame.returnI4(availableBytes);
    }

    // native void socketShutdown(int howto) throws IOException;
    void socketShutdown(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto howto = frame.getLocalI4(1);
        const auto sockfd = socketGetFdFieldValue(self);

        ::shutdown(sockfd, howto);
    }

    // native void socketSetOption0(int cmd, boolean on, Object value)
    void socketSetOption0(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto sockfd = socketGetFdFieldValue(self);
        const auto cmd = frame.getLocalI4(1);
        const auto on = frame.getLocalBoolean(2);
        const auto value = frame.getLocalRef(3);

        linger sLinger;
        sLinger.l_onoff = on ? 1 : 0;

        setsockopt(sockfd, SOL_SOCKET, cmd, (void*) &sLinger, sizeof(linger));
    }

    //native int socketGetOption(int opt, Object iaContainerObj) throws SocketException;
    void socketGetOption(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto sockfd = socketGetFdFieldValue(self);
        const auto opt = frame.getLocalI4(1);
        const auto iaContainerObj = frame.getLocalRef(2);

        linger sLinger;
        const auto lingerLen = sizeof(linger);
        getsockopt(sockfd, SOL_SOCKET, opt, (void*) &sLinger, (socklen_t *)(&lingerLen));
        frame.returnI4(sLinger.l_onoff);
    }

    //private native int socketRead0(FileDescriptor fd, byte b[], int off, int len, int timeout) throws IOException;
    void socketRead0(Frame &frame) {
        const auto fdOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(fdOop)
        const auto fdVal = fdOop->getFieldValue("fd" "I").i4Val;

        const auto b = CAST_BYTE_TYPE_ARRAY_OOP(frame.getLocalRef(2));
        ASSERT_IF_NULL_THROW_NPE(b)
        const auto off = frame.getLocalI4(3);
        const auto len = frame.getLocalI4(4);
        const auto timeout = frame.getLocalI4(5);

        auto bytePtr = reinterpret_cast<char*>(b->data.get());
        bytePtr += off;

        const auto ret = ::read(fdVal, bytePtr, len);
        if (ret == -1) {
            const auto errorMsg = std::strerror(errno);
            throwIOException(frame, "socket read error: " + cstring{errorMsg});
            return;
        } else if (ret == 0) {
            frame.returnI4(-1);
            return;
        } else {
            frame.returnI4(CAST_I4(ret));
            return;
        }
    }

    //private native void socketWrite0(FileDescriptor fd, byte[] b, int off, int len) throws IOException;
    void socketWrite0(Frame &frame) {
        const auto fdOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(fdOop)
        const auto fdVal = fdOop->getFieldValue("fd" "I").i4Val;

        const auto b = CAST_BYTE_TYPE_ARRAY_OOP(frame.getLocalRef(2));
        ASSERT_IF_NULL_THROW_NPE(b)
        const auto off = frame.getLocalI4(3);
        const auto len = frame.getLocalI4(4);

        const auto bytePtr = reinterpret_cast<const char*>(b->data.get());
        if (write(fdVal, bytePtr + off, len) == -1) {
            const auto errorMsg = std::strerror(errno);
            throwRuntimeException(frame, "socket write error: " + cstring{errorMsg});
            return;
        }
    }

    // public native String getLocalHostName() throws UnknownHostException;
    void getLocalHostName(Frame &frame) {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) < 0) {
            //error
            throwAssignException(frame, "java/net/UnknownHostException", "get host name fail");
        } else {
            const auto hostNameOop = frame.mem.getInternString(cstring{hostname});
            frame.returnRef(hostNameOop);
        }
    }

    void inetSsIPv6Supported(Frame &frame) {
        frame.returnBoolean(false);
    }
}


#endif