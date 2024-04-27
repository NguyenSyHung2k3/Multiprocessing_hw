#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include<time.h>

void signalHandler(int signo){
    pid_t pid = wait(NULL);
    printf("Child process terminated, pid = %d\n", pid);
}

int format_time(char *str){
    if(strcmp(str, "dd/mm/yyyy") == 0){
        return 1;
    } else if(strcmp(str, "dd/mm/yy") == 0){
        return 2;
    } else if(strcmp(str, "mm/dd/yyyy") == 0){
        return 3;
    } else if(strcmp(str, "mm/dd/yy") == 0){
        return 4;
    }
    return 0;
}

int main(){
     int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     if(listener == -1){
        perror("socket() failed");
        return 1;
     }

     // khai bao dia chi server
     struct sockaddr_in addr;
     addr.sin_family = AF_INET;
     addr.sin_addr.s_addr = htonl(INADDR_ANY);
     addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    if(listen(listener, 5)){
        perror("listen() failed");
        return 1;
    }

    signal(SIGCHLD, signalHandler);

     while(1){
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);
        
        if(fork() == 0){
            // Tien trinh con yeu cau xu ly tu client
            // Dong socket listener
            close(listener);

            char buf[256];
            while(1){
                int ret = recv(client, buf, sizeof(buf), 0);
                if(ret <= 0){
                    break;
                }

                buf[ret] = 0;
                printf("Received: %s", buf);

                if(buf[strlen(buf)-1] == '\n'){
                    buf[strlen(buf)-1] = 0;
                }

                char cmd[300];
                char f_cmd[300], s_cmd[300], tmp[300];
                strcpy(cmd, buf);
                int n = sscanf(cmd, "%s %s %s", f_cmd, s_cmd, tmp);

                struct tm *tm_time;
                time_t current_time;
                time(&current_time);
                tm_time = localtime(&current_time);
                char time_string[50];

                if(n != 2){
                    char *msg = "Sai cu phap. Hay nhap lai 0.\n";
                    send(client, msg, strlen(msg), 0);
                } else {
                    if(strcmp(f_cmd, "GET_TIME") != 0){
                        char *msg = "Sai cu phap. Hay nhap lai 1.\n";
                        send(client, msg, strlen(msg), 0);
                    } else {

                        if(format_time(s_cmd) == 1){
                            strftime(time_string, sizeof(time_string), "%d/%m/%Y", tm_time);
                            send(client, time_string, strlen(time_string), 0);
                        } else if (format_time(s_cmd) == 2){
                            strftime(time_string, sizeof(time_string), "%d/%m/%y", tm_time);
                            send(client, time_string, strlen(time_string), 0);
                        } else if (format_time(s_cmd) == 3){
                            strftime(time_string, sizeof(time_string), "%m/%d/%Y", tm_time);
                            send(client, time_string, strlen(time_string), 0);
                        } else if (format_time(s_cmd) == 4){
                            strftime(time_string, sizeof(time_string), "%m/%d/%y", tm_time);
                            send(client, time_string, strlen(time_string), 0);
                        } else {
                            char *msg = "Sai cu phap. Hay nhap lai 2.\n";
                            send(client, msg, strlen(msg), 0);
                        }
                    }
                }
            }

            // Ket thuc tien trinh con
            exit(0);
        }

        // Dong socket client o tien trinh cha
        close(client);
     }

     return 0;
}