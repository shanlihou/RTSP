#include "RTCP.h"

RTCP::RTCP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort) :UDP(srcPort + 1, dstAddr, dstPort + 1), 
    rtp(srcPort, dstAddr, dstPort)
{
    
}