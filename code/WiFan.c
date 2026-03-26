#include "stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "string.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "dhcpserver/dhcpserver.h"
#include "dnsserver/dnsserver.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "lwip/apps/httpd.h"

void httpd_init(void) ;

static absolute_time_t wifi_connected_time;
static bool led_on = false;
static bool setup_mode = true;
static char *ssid = "Your Wi-Fi SSID";
static char *password = "Your Wi-Fi Password";
static char *devicename = "WiFan Smart Fan";


int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    return 0;
}

static size_t get_mac_ascii(int idx, size_t chr_off, size_t chr_len, char *dest_in) {
    static const char hexchr[16] = "0123456789ABCDEF";
    uint8_t mac[6];
    char *dest = dest_in;
    assert(chr_off + chr_len <= (2 * sizeof(mac)));
    cyw43_hal_get_mac(idx, mac);
    for (; chr_len && (chr_off >> 1) < sizeof(mac); ++chr_off, --chr_len) {
        *dest++ = hexchr[mac[chr_off >> 1] >> (4 * (1 - (chr_off & 1))) & 0xf];
    }
    return dest - dest_in;
}

static const char *cgi_handler_test(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    if (iNumParams > 0) {
        if (strcmp(pcParam[0], "test") == 0) {
            return "/test.shtml";
        }
    }
    return "/index.shtml";
}

static tCGI cgi_handlers[] = {
    { "/", cgi_handler_test },
    { "/index.shtml", cgi_handler_test },
};

// Note that the buffer size is limited by LWIP_HTTPD_MAX_TAG_INSERT_LEN, so use LWIP_HTTPD_SSI_MULTIPART to return larger amounts of data
u16_t ssi_example_ssi_handler(int iIndex, char *pcInsert, int iInsertLen
#if LWIP_HTTPD_SSI_MULTIPART
    , uint16_t current_tag_part, uint16_t *next_tag_part
#endif
) {
    size_t printed;
    switch (iIndex) {
        case 0: { // "status"
            printed = snprintf(pcInsert, iInsertLen, "Pass");
            break;
        }
        case 1: { // "welcome"
            printed = snprintf(pcInsert, iInsertLen, "Hello from Pico");
            break;
        }
        case 2: { // "ledstate"
            printed = snprintf(pcInsert, iInsertLen, "%s", led_on ? "ON" : "OFF");
            break;
        }
        case 3: { // "ledinv"
            printed = snprintf(pcInsert, iInsertLen, "%s", led_on ? "OFF" : "ON");
            break;
        }
        default: { // unknown tag
            printed = 0;
            break;
        }
    }
  return (u16_t)printed;
}

// Be aware of LWIP_HTTPD_MAX_TAG_NAME_LEN
static const char *ssi_tags[] = {
    "status",
    "welcome",
    "ledstate",
    "ledinv",
};

#if LWIP_HTTPD_SUPPORT_POST
#define LED_STATE_BUFSIZE 4
static void *current_connection;

err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
        u16_t http_request_len, int content_len, char *response_uri,
        u16_t response_uri_len, u8_t *post_auto_wnd) {
    if (memcmp(uri, "/led.cgi", 8) == 0 && current_connection != connection) {
        current_connection = connection;
        snprintf(response_uri, response_uri_len, "/ledfail.shtml");
        *post_auto_wnd = 1;
        return ERR_OK;
    }
    return ERR_VAL;
}

// Return a value for a parameter
char *httpd_param_value(struct pbuf *p, const char *param_name, char *value_buf, size_t value_buf_len) {
    size_t param_len = strlen(param_name);
    u16_t param_pos = pbuf_memfind(p, param_name, param_len, 0);
    if (param_pos != 0xFFFF) {
        u16_t param_value_pos = param_pos + param_len;
        u16_t param_value_len = 0;
        u16_t tmp = pbuf_memfind(p, "&", 1, param_value_pos);
        if (tmp != 0xFFFF) {
            param_value_len = tmp - param_value_pos;
        } else {
            param_value_len = p->tot_len - param_value_pos;
        }
        if (param_value_len > 0 && param_value_len < value_buf_len) {
            char *result = (char *)pbuf_get_contiguous(p, value_buf, value_buf_len, param_value_len, param_value_pos);
            if (result) {
                result[param_value_len] = 0;
                return result;
            }
        }
    }
    return NULL;
}

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
    err_t ret = ERR_VAL;
    LWIP_ASSERT("NULL pbuf", p != NULL);
    if (current_connection == connection) {
        char buf[LED_STATE_BUFSIZE];
        char *val = httpd_param_value(p, "led_state=", buf, sizeof(buf));
        if (val) {
            led_on = (strcmp(val, "ON") == 0) ? true : false;
            cyw43_gpio_set(&cyw43_state, 0, led_on);
            ret = ERR_OK;
        }
    }
    pbuf_free(p);
    return ret;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    snprintf(response_uri, response_uri_len, "/ledfail.shtml");
    if (current_connection == connection) {
        snprintf(response_uri, response_uri_len, "/ledpass.shtml");
    }
    current_connection = NULL;
}
#endif

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Timer example code - This example fires off the callback after 2000ms
    add_alarm_in_ms(2000, alarm_callback, NULL, false);
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    // Set up the hostname based on the device MAC address (so it's unique for each device)
    add_compile_definitions(CYW43_HOST_NAME="WiFan")
    char hostname[sizeof(CYW43_HOST_NAME) + 4];
    memcpy(&hostname[0], CYW43_HOST_NAME, sizeof(CYW43_HOST_NAME) - 1);
    get_mac_ascii(CYW43_HAL_MAC_WLAN0, 8, 4, &hostname[sizeof(CYW43_HOST_NAME) - 1]);
    hostname[sizeof(hostname) - 1] = '\0';
    netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], hostname);

    // Setup http server
    cyw43_arch_lwip_begin();

    if (setup_mode) 
    {
        printf("Entering initial setup mode\n");
        // In initial setup mode, we start a Wi-Fi access point so the user can configure:
        // - Wi-Fi SSID
        // - Wi-Fi WPA2 password to connect
        // - Device name (for mDNS and Home Assistant auto discovery)
        const char *ap_name = "WiFan-Setup";
        const char *password = NULL;

        cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

        #if LWIP_IPV6
        #define IP(x) ((x).u_addr.ip4)
        #else
        #define IP(x) (x)
        #endif

        ip4_addr_t mask;
        IP(state->gw).addr = PP_HTONL(LWIP_MAKEU32(192, 168, 50, 1));
        IP(mask).addr = PP_HTONL(CYW43_DEFAULT_IP_MASK);

        #undef IP

        // Start the dhcp server
        dhcp_server_t dhcp_server;
        dhcp_server_init(&dhcp_server, &state->gw, &mask);

        // Start the dns server
        dns_server_t dns_server;
        dns_server_init(&dns_server, &state->gw);

        if (!tcp_server_open(state, ap_name)) {
            DEBUG_printf("Failed to start DHCP/WiFi server\n");
            return 1;
        }
        // Now we have the network set up, we can start the http server and handle requests
        httpd_init();
        http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
        http_set_ssi_handler(ssi_example_ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
        cyw43_arch_lwip_end();

        while(true) {
        #if PICO_CYW43_ARCH_POLL
                cyw43_arch_poll();
                cyw43_arch_wait_for_work_until(led_time);
        #else
                sleep_ms(1000);
        #endif
        }
        // Cleanup after the user has finished the initial setup
        dns_server_deinit(&dns_server);
        dhcp_server_deinit(&dhcp_server);
        cyw43_arch_deinit();
    }
    else
    {
        // Enable wifi station
        cyw43_arch_enable_sta_mode();

        printf("Connecting to Wi-Fi...\n");
        if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            printf("WiFanfailed to connect.\n");
            return 1;
        } else {
            printf("Connected.\n");
            // Read the ip address in a human readable way
            uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
            printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        }
    }

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
