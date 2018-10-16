//
// Created by senaneuc on 12/06/18.
//

#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#define NB_CONNECTION_MAX 5

#include <iostream>
#include <vector>

using namespace std;

class TcpServer {
public:
    TcpServer();
    virtual ~TcpServer();

    int Listen (int port);
    int AcceptClient();
    int Send(int clien_fd, string mes);
    int Broadcast(string mes);
    string Receive(int client_fd, int size);

    const vector<int> &getSocketClients() const;

    void SetSocketClients(const vector<int> &socketClients);

private:
    vector<int> socketClients;
    int socketFD;
};

#endif /* TCP_SERVER_H_ */
