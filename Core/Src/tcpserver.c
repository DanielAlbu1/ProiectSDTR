#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "string.h"
#include "stdio.h"
#include "tcpserver.h"
#include "queue_manager.h"
static struct netconn *conn, *newconn;
static struct netbuf *buf;
char msg[512];
char smsg[1024];

 float get_humidity_value(void)
{
    HumiditySensorData sensor_data;

    if (xQueuePeek(sensorQueue, &sensor_data, 0) == pdTRUE)
    {
        return sensor_data.humidity;
    }
    return -1.0f;
}

static void process_http_request(const char *request, char *response)
{
    if (strstr(request, "GET / HTTP/1.1") != NULL)
    {
        float humidity = get_humidity_value();

        char humidity_str[100];
        snprintf(humidity_str, sizeof(humidity_str), "%.2f", humidity);

        const char *http_response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n";
        strcpy(response, http_response);
        strcat(response, humidity_str);
    }
    else if (strstr(request, "POST / HTTP/1.1") != NULL)
    {

        const char *body = strstr(request, "\r\n\r\n");
        if (body != NULL)
        {
            body += 4;

            LedMessage message;
            if (strstr(body, "led=1") != NULL)
            {
                message.led = LED1;
            }
            else if (strstr(body, "led=2") != NULL)
            {
                message.led = LED2;
            }
            else if (strstr(body, "led=3") != NULL)
            {
                message.led = LED3;
            }
            else
            {
                snprintf(response, 256, "HTTP/1.1 400 Bad Request\r\n\r\nInvalid LED ID");
                return;
            }

            if (strstr(body, "toggle=1") != NULL)
            {
                message.toggle = 1;
            }
            else
            {
                snprintf(response, 256, "HTTP/1.1 400 Bad Request\r\n\r\nMissing toggle value");
                return;
            }

            if (xQueueSendToBack(ledQueue, &message, 0) != pdTRUE)
            {
                snprintf(response, 256, "HTTP/1.1 500 Internal Server Error\r\n\r\nQueue full");
                return;
            }

            snprintf(response, 256, "HTTP/1.1 200 OK\r\n\r\nLED %d toggled", message.led);
        }
        else
        {
            snprintf(response, 256, "HTTP/1.1 400 Bad Request\r\n\r\nNo body found");
        }
    }
    else
    {
        const char *http_response =
            "HTTP/1.1 404 Not Found\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>404 Not Found</h1></body></html>";
        strcpy(response, http_response);
    }
}



static void tcp_thread(void *arg)
{
    err_t err, accept_err;


    conn = netconn_new(NETCONN_TCP);

    if (conn != NULL)
    {

        err = netconn_bind(conn, IP_ADDR_ANY, 80);

        if (err == ERR_OK)
        {

            netconn_listen(conn);

            while (1)
            {
            	//PrintTaskTiming("TCP_start");
                accept_err = netconn_accept(conn, &newconn);

                if (accept_err == ERR_OK)
                {

                    while (netconn_recv(newconn, &buf) == ERR_OK)
                    {

                        strncpy(msg, buf->p->payload, buf->p->len);


                        //printf("Received request: %s\n", msg);

                        process_http_request(msg, smsg);

                        netconn_write(newconn, smsg, strlen(smsg), NETCONN_COPY);

                        netbuf_delete(buf);

                        netconn_close(newconn);
                        netconn_delete(newconn);
                    }
                }
                else
                {
                    printf("Error accepting connection\n");
                }
             //   PrintTaskTiming("TCP_end");
            }
        }
        else
        {
            printf("Error binding to port 80\n");
            netconn_delete(conn);
        }
    }
}


void tcpserver_init(void)
{
    sys_thread_new("tcp_thread", tcp_thread, NULL, DEFAULT_THREAD_STACKSIZE, osPriorityRealtime);
}
