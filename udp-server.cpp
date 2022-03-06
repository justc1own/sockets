#include <winsock2.h>
#include <cstdio>
#include <conio.h>

#include <set>
#include <map>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;

#define SERVER_PORT  1234
#define ONE_MS       1
#define ESC_KEY      27
#define ENTER_KEY    13

#define MAX_BUF_SIZE 2048

SOCKET sock;
sockaddr_in address;
int address_len;

char buf_out[MAX_BUF_SIZE];
char buf_in[MAX_BUF_SIZE];

int buf_out_len;
int buf_in_len;

bool operator<(const sockaddr_in &a, const sockaddr_in &b) {
    return a.sin_addr.s_addr < b.sin_addr.s_addr || 
    (a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port < b.sin_port);
}

set<sockaddr_in> guest;
map<sockaddr_in, string> name;

string s;


int init_sockets (void) {
    WORD    version; // short - 16 bit
    WSADATA wsaData;
    int     err;
    int     option_value;

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
    address.sin_addr.s_addr = INADDR_ANY;//inet_addr("192.168.56.1");//INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);


    if (bind(sock, (sockaddr *) &address, sizeof(sockaddr)) == SOCKET_ERROR) {
        fprintf(stderr, "can't bind socket\n");
        closesocket (sock);
        return SOCKET_ERROR;
    }

    return 0;
}

void dbg_name() {
    cout << "== START == " << endl;
    for(map<sockaddr_in, string>::iterator it = name.begin(); it != name.end(); ++it) {
        cout << "address = " << it->first.sin_addr.s_addr << " name=" << it->second << endl;
        //cout << "address = " << it->first << " name=" << it->second << endl;
    }
    cout << "== FINISH == " << endl << endl;
}


int main() {
    if (init_sockets()) return 0;

    while (true) {
        address_len = sizeof(address);
        buf_in_len = recvfrom(sock, buf_in, sizeof(buf_in), 0, (sockaddr *) &address, &address_len);
        

        
        if (buf_in_len >= 0) {
            printf("buf_in_len %i\n", buf_in_len);
            buf_in[ buf_in_len ] = 0;            
            s = buf_in;


            guest.insert(address);


            if (s.substr(0, 6) == "NAME: " || s.substr(0, 6) == "name: " || s.substr(0, 6) == "Name: ") {

                string w = s.substr(6, s.size() - 6);

                name[address] = w;
                continue;
            }



            if (name.find(address) == name.end()) {
                s = "unknown: ";
                s = s + buf_in;                
            } else {
                s = name[address] + ": " + buf_in;
            }

            
            for(set<sockaddr_in>::iterator it = guest.begin(); it != guest.end(); ++it) {
                sendto(sock, s.c_str(), s.size(), 0, (sockaddr*) &(*it), sizeof(sockaddr_in));
            }
            
        }
    }

    closesocket(sock);
}