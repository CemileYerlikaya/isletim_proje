#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "ota-metadata.h"
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "sys/node-id.h"
#include "sys/log.h"
#include "firmware_data.h"
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

#define SEND_INTERVAL		  (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;
static ota_boot_metadata_t boot_metadata = {
  .magic = OTA_IMAGE_MAGIC,
  .active_slot = OTA_SLOT_A,
  .candidate_slot = OTA_SLOT_NONE,
  .state_a = OTA_IMAGE_STATE_CONFIRMED,
  .state_b = OTA_IMAGE_STATE_EMPTY,
};
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  static uint32_t fake_version = 2;
  uint32_t fake_image_crc;

  (void)c;
  (void)sender_port;
  (void)receiver_addr;
  (void)receiver_port;

  LOG_INFO("Client received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
  rx_count++;

  fake_image_crc = ota_crc32_buffer(data, datalen);
  if(ota_metadata_mark_verified(&boot_metadata, OTA_SLOT_B,
                                fake_version, datalen, fake_image_crc) &&
     ota_metadata_stage_verified_image(&boot_metadata, OTA_SLOT_B)) {
    LOG_INFO("OTA metadata updated: slot B staged for activation\n");
  }
}
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;
  static uint32_t tx_count;
  static uint32_t missed_tx_count;

  PROCESS_BEGIN();
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() &&
        NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

      if(node_id == 2) {

        LOG_INFO("Sending request %" PRIu32 " to ", tx_count);
        LOG_INFO_6ADDR(&dest_ipaddr);
        LOG_INFO_("\n");

        snprintf(str, sizeof(str), "Merhaba %" PRIu32, tx_count);
        if(node_id == 2) { 
          static uint16_t p_id = 0;
          struct ota_packet_t packet;
          uint32_t total_bytes = sizeof(new_firmware_data);
          uint32_t offset = p_id * 64;
      
          if(offset < total_bytes) {
              packet.packet_id = p_id;
              uint8_t len = (total_bytes - offset > 64) ? 64 : (total_bytes - offset);
              packet.length = len;
      
              memcpy(packet.data, &new_firmware_data[offset], len);
              packet.checksum = 0; 
              for(uint8_t i=0; i<len; i++) packet.checksum ^= packet.data[i];
      
              uip_ipaddr_t dest_ipaddr;
              uip_ip6addr(&dest_ipaddr, 0xfe80, 0, 0, 0, 0x0201, 0x0001, 0x0001, 0x0001);
      
              simple_udp_sendto(&udp_conn, &packet, sizeof(packet), &dest_ipaddr);
              p_id++;
          }
      }
        tx_count++;
      }


    } else {
      LOG_INFO("Not reachable yet\n");
      if(tx_count > 0) {
        missed_tx_count++;
      }
    }
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
