#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "string.h"
#include "stdio.h"
#include "queue_manager.h"
static struct netconn *conn, *newconn;
static struct netbuf *buf;
char msg[512];
char smsg[1024];

static float get_humidity_value(void)
{
    HumiditySensorData sensor_data;
    // Verificăm dacă valoarea există în coadă
    if (xQueuePeek(sensorQueue, &sensor_data, 0) == pdTRUE)
    {
        return sensor_data.humidity; // Returnăm valoarea umidității
    }
    return -1.0f; // Dacă nu sunt date, returnăm o valoare invalidă
}
// Funcție pentru a analiza cererea HTTP
static void process_http_request(const char *request, char *response)
{
    if (strstr(request, "GET / HTTP/1.1") != NULL)
    {
        // Obținem valoarea umidității
        float humidity = get_humidity_value();

        // Pregătește răspunsul
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
        // Găsește corpul cererii
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


// Funcția principală a serverului TCP
static void tcp_thread(void *arg)
{
    err_t err, accept_err;

    // Creează o conexiune TCP
    conn = netconn_new(NETCONN_TCP);

    if (conn != NULL)
    {
        // Leagă conexiunea de portul 80 (HTTP default)
        err = netconn_bind(conn, IP_ADDR_ANY, 80);

        if (err == ERR_OK)
        {
            // Intră în modul de ascultare
            netconn_listen(conn);

            while (1)
            {
                // Acceptă o nouă conexiune
                accept_err = netconn_accept(conn, &newconn);

                if (accept_err == ERR_OK)
                {
                    // Primește date de la client
                    while (netconn_recv(newconn, &buf) == ERR_OK)
                    {
                        // Procesează cererea HTTP
                        strncpy(msg, buf->p->payload, buf->p->len);  // Extrage cererea

                        // Print pentru debug
                        printf("Received request: %s\n", msg);

                        // Pregătește răspunsul HTTP
                        process_http_request(msg, smsg);

                        // Trimite răspunsul la client
                        netconn_write(newconn, smsg, strlen(smsg), NETCONN_COPY);

                        // Curăță bufferul
                        netbuf_delete(buf);

                        // Închide și șterge conexiunea după fiecare răspuns
                        netconn_close(newconn);
                        netconn_delete(newconn);
                    }
                }
                else
                {
                    printf("Error accepting connection\n");
                }
            }
        }
        else
        {
            printf("Error binding to port 80\n");
            netconn_delete(conn);
        }
    }
}

// Funcția de inițializare
void tcpserver_init(void)
{
    sys_thread_new("tcp_thread", tcp_thread, NULL, DEFAULT_THREAD_STACKSIZE, osPriorityNormal);
}
