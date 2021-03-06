//
// Created by Killua on 2018/2/4.
//


#include "MyRoboCatPCH.h"


bool SocketUtil::StaticInit() {
    return true;
}

void SocketUtil::CleanUp() {

}

void SocketUtil::ReportError(const char *inOperationDesc) {
    printf("Error: %s", inOperationDesc);
}

int SocketUtil::GetLastError() {
    return errno;
}


UDPSocketPtr SocketUtil::CreateUDPSocket(SocketAddressFamily inFamily) {
    SOCKET s = socket(inFamily, SOCK_DGRAM, IPPROTO_UDP);

    if (s != INVALID_SOCKET) {
        return UDPSocketPtr(new UDPSocket(s));
    }
}


TCPSocketPtr SocketUtil::CreateTCPSocket(SocketAddressFamily inFamily) {
    SOCKET s = socket(inFamily, SOCK_STREAM, IPPROTO_TCP);

    if (s != INVALID_SOCKET) {
        return TCPSocketPtr(new TCPSocket(s));
    } else {
        ReportError("Socketutil::createTcpSocket");
        return nullptr;
    }
}

fd_set *SocketUtil::FillSetFromVector(fd_set &outSet, const std::vector<TCPSocketPtr> *inSockets, int &ioNaxNfds) {
    if (inSockets) {
        FD_ZERO(&outSet);
        for (const TCPSocketPtr &socket : *inSockets) {
            FD_SET(socket->mSocket, &outSet);
            ioNaxNfds = std::max(ioNaxNfds, socket->mSocket);
        }
        return &outSet;
    } else {
        return nullptr;
    }
}

void SocketUtil::FillVectorFromSet(std::vector<TCPSocketPtr> *outSockets, const std::vector<TCPSocketPtr> *inSockets,
                                   const fd_set &inSet) {
    if (inSockets && outSockets) {
        outSockets->clear();
        for (const TCPSocketPtr &socket : *inSockets) {
            if (FD_ISSET(socket->mSocket, &inSet)) {
                outSockets->push_back(socket);
            }
        }
    }
}

int SocketUtil::Select(const std::vector<TCPSocketPtr> *inReadSet, std::vector<TCPSocketPtr> *outReadSet,
                       const std::vector<TCPSocketPtr> *inWriteSet, std::vector<TCPSocketPtr> *outWriteSet,
                       const std::vector<TCPSocketPtr> *inExceptSet, std::vector<TCPSocketPtr> *outExceptSet) {
    fd_set read, write, except;

    int nfds = 0;

    fd_set *readPtr = FillSetFromVector(read, inReadSet, nfds);
    fd_set *writePtr = FillSetFromVector(read, inWriteSet, nfds);
    fd_set *exceptPtr = FillSetFromVector(read, inExceptSet, nfds);

    int toRet = select(nfds + 1, readPtr, writePtr, exceptPtr, nullptr);

    if (toRet > 0) {
        FillVectorFromSet(outReadSet, inReadSet, read);
        FillVectorFromSet(outWriteSet, inWriteSet, write);
        FillVectorFromSet(outExceptSet, inExceptSet, except);
    }
}
