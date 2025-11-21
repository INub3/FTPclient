/* TCPftp.c - Cliente FTP concurrente (GET/PUT simultáneos) */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern int errno;

int errexit(const char *format, ...);
int connectTCP(const char *host, const char *service);
int passiveTCP(const char *service, int qlen);

#define LINELEN 128

/* =============================== */
/* Envia comandos FTP y muestra respuesta */
void sendCmd(int s, char *cmd, char *res) {
    int n;
    n = strlen(cmd);
    cmd[n] = '\r';
    cmd[n+1] = '\n';
    n = write(s, cmd, n+2);
    n = read(s, res, LINELEN);
    res[n] = '\0';
    printf("%s\n", res);
}

/* =============================== */
/* Modo pasivo: obtiene IP/puerto y conecta */
int pasivo(int s) {
    int sdata;
    int nport;
    char cmd[128], res[128], *p;
    char host[64], port[8];
    int h1,h2,h3,h4,p1,p2;

    sprintf(cmd, "PASV");
    sendCmd(s, cmd, res);
    p = strchr(res, '(');
    sscanf(p+1, "%d,%d,%d,%d,%d,%d", &h1,&h2,&h3,&h4,&p1,&p2);
    snprintf(host, 64, "%d.%d.%d.%d", h1,h2,h3,h4);
    nport = p1*256 + p2;
    snprintf(port, 8, "%d", nport);
    sdata = connectTCP(host, port);
    return sdata;
}

/* =============================== */
/* Ayuda */
void ayuda() {
    printf("Cliente FTP. Comandos disponibles:\n \
    help        - muestra este texto\n \
    dir         - lista el directorio actual del servidor\n \
    get <archivo> - descarga un archivo del servidor\n \
    put <archivo> - sube un archivo al servidor\n \
    cd <dir>    - cambia de directorio en el servidor\n \
    pwd         - muestra directorio actual del servidor\n \
    delete <archivo> - elimina archivo en servidor\n \
    rename <old> <new> - renombra archivo en servidor\n \
    quit        - finaliza la sesión FTP\n\n");
}

/* =============================== */
/* Conecta y autentica (reutilizable para hijos) */
int ftp_connect_and_login(const char *host, const char *service, const char *user, const char *pass) {
    int s = connectTCP(host, service);
    char cmd[128], res[128];
    int n;

    n = read(s, res, LINELEN);
    res[n] = '\0';

    sprintf(cmd, "USER %s", user);
    sendCmd(s, cmd, res);
    sprintf(cmd, "PASS %s", pass);
    sendCmd(s, cmd, res);

    if (strncmp(res, "230", 3) != 0) {
        fprintf(stderr, "Error de autenticación.\n");
        close(s);
        return -1;
    }
    return s;
}

/* =============================== */
/* Función para comando GET */
void comando_get(int s_child, char *arg) {
    int sdata, n;
    char cmd[128], res[128], data[LINELEN];
    FILE *fp;

    sdata = pasivo(s_child);
    sprintf(cmd, "RETR %s", arg);
    sendCmd(s_child, cmd, res);
    if (strncmp(res, "150", 3) != 0) exit(1);

    fp = fopen(arg, "wb");
    if (!fp) { perror("fopen"); exit(1); }

    while ((n = recv(sdata, data, LINELEN, 0)) > 0) {
        fwrite(data, 1, n, fp);
        usleep(200000); // pausa de 200 ms (0.2 segundos)
    }

    fclose(fp);
    close(sdata);

    n = read(s_child, res, LINELEN);
    res[n] = '\0';
    printf("[GET %s terminado] %s\n", arg, res);
}

/* =============================== */
/* Función para comando PUT */
void comando_put(int s_child, char *arg) {
    int sdata, n;
    char cmd[128], res[128], data[LINELEN];
    FILE *fp;

    fp = fopen(arg, "rb");
    if (!fp) { 
        perror("fopen"); 
        exit(1); 
    }

    sdata = pasivo(s_child);
    sprintf(cmd, "STOR %s", arg);
    sendCmd(s_child, cmd, res);

    if (strncmp(res, "150", 3) != 0) {
        fclose(fp);
        close(sdata);
        exit(1);
    }

    while ((n = fread(data, 1, LINELEN, fp)) > 0) {
        send(sdata, data, n, 0);
    }

    fclose(fp);
    close(sdata);

    n = read(s_child, res, LINELEN);
    res[n] = '\0';
    printf("[PUT %s terminado] %s\n", arg, res);
}

/* =============================== */
/* Función para comando DELETE */
void comando_delete(int s, char *arg) {
    char cmd[128], res[128];
    sprintf(cmd, "DELE %s", arg);
    sendCmd(s, cmd, res);
}

/* =============================== */
/* Función para comando RENAME */
void comando_rename(int s, char *old_name, char *new_name) {
    char cmd[128], res[128];
    
    sprintf(cmd, "RNFR %s", old_name);
    sendCmd(s, cmd, res);
    
    if (strncmp(res, "350", 3) == 0) {
        sprintf(cmd, "RNTO %s", new_name);
        sendCmd(s, cmd, res);
    } else {
        printf("Error en renombrado: %s\n", res);
    }
}

/* =============================== */
/* Función para comando PWD */
void comando_pwd(int s) {
    char cmd[128], res[128];
    sprintf(cmd, "PWD");
    sendCmd(s, cmd, res);
}

/* =============================== */
int main(int argc, char *argv[]) {
    char *host = "localhost";
    char *service = "ftp";
    char cmd[128], res[128];
    char prompt[64], *ucmd, *arg, *arg2;
    char user[32];
    char *pass;
    int s, n;

    /* Parámetros */
    switch (argc) {
        case 1: host = "localhost"; break;
        case 3: service = argv[2];
        case 2: host = argv[1]; break;
        default:
            fprintf(stderr, "Uso: TCPftp [host [port]]\n");
            exit(1);
    }

    /* Conexión inicial */
    s = connectTCP(host, service);
    n = read(s, res, LINELEN);
    res[n] = '\0';
    printf("%s\n", res);

    /* Autenticación */
    while (1) {
        printf("Please enter your username: ");
        scanf("%s", user);
        sprintf(cmd, "USER %s", user);
        sendCmd(s, cmd, res);

        pass = getpass("Enter your password: ");
        sprintf(cmd, "PASS %s", pass);
        sendCmd(s, cmd, res);
        if (strncmp(res, "230", 3) == 0) break;
    }

    fgets(prompt, sizeof(prompt), stdin); // limpiar buffer
    ayuda();

    /* Bucle principal */
    while (1) {
        printf("ftp> ");
        if (fgets(prompt, sizeof(prompt), stdin) == NULL)
            break;

        prompt[strcspn(prompt, "\n")] = 0;
        ucmd = strtok(prompt, " ");
        if (!ucmd) continue;

        if (strcmp(ucmd, "dir") == 0) {
            int sdata = pasivo(s);
            sprintf(cmd, "LIST");
            sendCmd(s, cmd, res);
            
            char data[LINELEN+1];
            while ((n = recv(sdata, data, LINELEN, 0)) > 0) {
                fwrite(data, 1, n, stdout);
            }
            
            close(sdata);
            n = read(s, res, LINELEN);
            res[n] = '\0';
            printf("%s\n", res);

        } else if (strcmp(ucmd, "get") == 0 || strcmp(ucmd, "put") == 0) {
            arg = strtok(NULL, " ");
            if (!arg) {
                printf("Uso: %s <archivo>\n", ucmd);
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                continue;
            }
            else if (pid == 0) {
                /* ====== HIJO ====== */
                int s_child = ftp_connect_and_login(host, service, user, pass);
                if (s_child < 0) exit(1);

                if (strcmp(ucmd, "get") == 0) {
                    comando_get(s_child, arg);
                } else {
                    comando_put(s_child, arg);
                }

                close(s_child);
                exit(0);
            }
            else {
                printf("%s '%s' en segundo plano (PID=%d)\n", 
                       strcmp(ucmd,"get")==0?"Descargando":"Subiendo", arg, pid);
            }

        } else if (strcmp(ucmd, "cd") == 0) {
            arg = strtok(NULL, " ");
            if (!arg) {
                printf("Uso: cd <directorio>\n");
                continue;
            }
            sprintf(cmd, "CWD %s", arg);
            sendCmd(s, cmd, res);

        } else if (strcmp(ucmd, "pwd") == 0) {
            comando_pwd(s);

        } else if (strcmp(ucmd, "delete") == 0) {
            arg = strtok(NULL, " ");
            if (!arg) {
                printf("Uso: delete <archivo>\n");
                continue;
            }
            comando_delete(s, arg);

        } else if (strcmp(ucmd, "rename") == 0) {
            arg = strtok(NULL, " ");
            arg2 = strtok(NULL, " ");
            if (!arg || !arg2) {
                printf("Uso: rename <archivo_viejo> <archivo_nuevo>\n");
                continue;
            }
            comando_rename(s, arg, arg2);

        } else if (strcmp(ucmd, "help") == 0) {
            ayuda();

        } else if (strcmp(ucmd, "quit") == 0) {
            sprintf(cmd, "QUIT");
            sendCmd(s, cmd, res);
            close(s);
            exit(0);

        } else {
            printf("%s: comando no implementado.\n", ucmd);
        }
    }

    close(s);
    return 0;
}
