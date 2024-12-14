#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "string.h"
#include "stdio.h"

static struct netconn *conn, *newconn;
static struct netbuf *buf;
char msg[512];
char smsg[1024];

// Funcție pentru a analiza cererea HTTP
static void process_http_request(const char *request, char *response)
{
    if (strstr(request, "GET / HTTP/1.1") != NULL)
    {
        // Răspuns pentru cererea GET
        const char *http_response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>Hello from STM32 HTTP Server</h1></body></html>";
        strcpy(response, http_response);
    }
    else if (strstr(request, "POST / HTTP/1.1") != NULL)
    {
        // Răspuns pentru cererea POST
        const char *http_response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>Data received via POST!</h1></body></html>";
        strcpy(response, http_response);
    }
    else
    {
        // Răspuns pentru alte cereri (404 Not Found)
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
