#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"
#include "storage/cfs/cfs.h"

struct ota_packet_t {
    uint16_t packet_id;
    uint8_t length;
    uint16_t checksum;
    uint8_t data[64];
};
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  struct ota_packet_t *m = (struct ota_packet_t *)data;

  int fd = cfs_open("new-firmware.z1", CFS_WRITE | CFS_APPEND);
  if(fd >= 0) {
      cfs_write(fd, m->data, m->length);
      cfs_close(fd);
  }
  
  if(m->packet_id >= 2020) { 
      printf("Yuklenmeye hazir yeni firmware alimi tamamlandi.\n");
  }
}
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_ROUTING.root_start();

  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
