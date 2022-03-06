#include <winsock2.h>
#include <cstdio>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <string>

using namespace std;

#define HEIGHT 40

#define SERVER_PORT  1234
#define ONE_MS       20
#define ESC_KEY      27
#define ENTER_KEY    13

#define MAX_BUF_SIZE 2048

SOCKET sock;
sockaddr_in address;
sockaddr_in address_in;

int address_len;
int timestamp;
int option_value;

char buf_out[MAX_BUF_SIZE];
char buf_in[MAX_BUF_SIZE];

int buf_out_len;
int buf_in_len;

char empty[] = "                                                                       ";
vector<string> v;

void SetTextColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void SetCursorPosition(int y, int x) {
    COORD cursor = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}

void SetCursorVisibility(bool show) {
    CONSOLE_CURSOR_INFO cursor;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
    cursor.bVisible = show;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
}



int init_sockets (void) {
    WORD    version; // short - 16 bit
    WSADATA wsaData;
    int     err;


    version = MAKEWORD(2, 2);

    err = WSAStartup(version, &wsaData);

    if (err != 0)  {
        fprintf(stderr, "Can't find usable WinSock DLL! \n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock == SOCKET_ERROR) {
        fprintf(stderr, "can't create socket\n");
        return 1;
    }


    address.sin_family = AF_INET;
    address.sin_addr.S_un.S_addr = inet_addr("192.168.19.159");
    address.sin_port = htons(SERVER_PORT);

    return 0;
}

int is_socket_ready() {
    fd_set fds ;
    int n ;
    struct timeval tv ;

    // Set up the file descriptor set.
    FD_ZERO(&fds) ;
    FD_SET(sock, &fds) ;

    // Set up the struct timeval for the timeout.
    tv.tv_sec = 0 ;
    tv.tv_usec = 10000;

    // Wait until timeout or data received.
    n = select (sock, &fds, NULL, NULL, &tv ) ;

    return n > 0;
}

void nice() {
    // clear
    for(int i = 0; i < HEIGHT - 1; ++i) {
        SetCursorPosition(i, 0);
        puts(empty);
    }

    int a = max((int)0, (int)(v.size() - 1 - HEIGHT + 3));
    int b = v.size();

    for(int i = a, j = 1; i < b; ++i, ++j) {
        SetCursorPosition(j, 0);
        SetTextColor(10);
        printf("%s\n", v[i].c_str());
        SetTextColor(7);

    }
}


int main() {
    SetTextColor(7);
    init_sockets();

    nice();

    SetCursorVisibility(0);

    for(int i = 0; i < 1000; ++i) {
        puts(empty);
    }

    SetCursorPosition(HEIGHT - 1, 0);
    puts("--------------------------------------------------------------");

    SetCursorPosition(10, 1);

    while (true) {
        
        if (_kbhit()) {
            int c = _getch();

            if (c == ESC_KEY) break;

            buf_out[buf_out_len++] = c;

            if (c == ENTER_KEY) {

                buf_out_len--;
                sendto(sock, buf_out, buf_out_len, 0, (sockaddr *)&address, sizeof(address));
                buf_out_len = 0;
                SetCursorPosition(HEIGHT, buf_out_len);
                puts(empty);
                                                      
            } else {
                
                SetCursorPosition(HEIGHT, buf_out_len - 1);
                printf("%c", c);
            }           
            
        }

        if (is_socket_ready() > 0) {

            address_len = sizeof(address);
            buf_in_len = recvfrom(sock, buf_in, sizeof(buf_in), 0, (sockaddr *) &address_in, &address_len);

            if (buf_in_len >= 0) {
                buf_in[ buf_in_len ] = 0;
                v.push_back(buf_in);
                /*
                SetTextColor(10);
                printf("%s\n", buf_in);
                SetTextColor(7);
                */

                nice();
            }            

        }


    }
}