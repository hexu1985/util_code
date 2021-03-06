/* Simple ARP sniffer
 * To compile: gcc tcpsniffer.c -o tcppsniffer -lpcap
 * Run as root!
 * */
#include "httpreq.h"
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> /* includes net/ethernet.h */
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <iostream>

using std::cout;
using std::endl;

#define MAXBYTES2CAPTURE 2048
int main(int argc, char *argv[])
{
	bpf_u_int32 netaddr=0, mask=0;
	struct bpf_program filter;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *descr = NULL;
	struct pcap_pkthdr *pkthdr;
	const unsigned char *packet = NULL;
	struct ip *ipptr = NULL;
	struct tcphdr *tcpptr = NULL;
	memset(errbuf, 0, PCAP_ERRBUF_SIZE);

	if (argc != 2){
		printf("Usage: ipsniffer <interface>\n");
		exit(1);
	}

	descr = pcap_open_live(argv[1], MAXBYTES2CAPTURE, 0,
			0, errbuf);
	pcap_lookupnet(argv[1], &netaddr, &mask, errbuf);
	pcap_compile(descr, &filter, "tcp and dst port 80", 1, mask);
	pcap_setfilter(descr, &filter);

	Httpreq_header httpreq_hdr;
	while (1){
		int ret = pcap_next_ex(descr, &pkthdr, &packet);
		if (ret < 0) {
			printf("pcap_next_ex error: %s\n", pcap_geterr(descr));
			exit(1);
		}
		if (packet == NULL)
			continue;

		ipptr = (struct ip *)(packet + 14);

		tcpptr = (struct tcphdr *) (packet + 14 + ipptr->ip_hl*4);

		if (!httpreq_parse((uint8_t *) packet + 14 + ipptr->ip_hl*4 + tcpptr->th_off*4, 
				(uint8_t *) packet + pkthdr->caplen, &httpreq_hdr)) {
			continue;
		}

		printf("Received Packet Size: %d bytes\n", pkthdr->len);
		printf("Capture Packet Size: %d bytes\n", pkthdr->caplen);
		printf("the IP packets version: %d\n", ipptr->ip_v); 
		printf ("the IP packets total_length is :%d\n", ntohs(ipptr->ip_len));
		printf ("the IP protocol is %d\n", ipptr->ip_p);
		printf ("Destination IP: %s\n", inet_ntoa(ipptr->ip_dst));    
		printf ("Source IP: %s\n", inet_ntoa(ipptr->ip_src));

		printf ("Destination port : %d\n", ntohs(tcpptr->th_dport));
		printf ("Source port : %d\n", ntohs(tcpptr->th_sport));
		printf ("the seq of packet is %u\n", ntohl(tcpptr->th_seq));

		cout << "HTTP version: " << httpreq_hdr.version << '\n';
		cout << "HTTP method: " << httpreq_hdr.method << '\n';
		cout << "HTTP uri: " << httpreq_hdr.uri << '\n';
		cout << "HTTP host: " << httpreq_hdr.host << '\n';
		cout << "HTTP user_agent: " << httpreq_hdr.user_agent << '\n';
		cout << endl;
	}
	return 0;
}

