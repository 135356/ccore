#include <iostream>
#include "cc/net/IpTcp.h"

int main(int argc, char **argv) {
    cc::net::IpTcp("127.0.0.1",80,4);
    return 0;
}