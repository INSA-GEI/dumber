/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "message.h"

void set_msgToMon_header(MessageToMon *msg, char *header) {
    memcpy(msg->header, header, sizeof (header));
}

void set_msgToMon_data(MessageToMon *msg, void * data) {
    if (msg->data != NULL) {
        free(msg->data);
    }
    msg->data = malloc(sizeof (data));
    memcpy(msg->data, data, sizeof (data));
}

void free_msgToMon_data(MessageToMon *msg) {
    if (msg->data != NULL) {
        free(msg->data);
    }
}

void print_msgToMon(MessageToMon *msg) {
    printf("header: %s\n", msg->header);
    printf("data: %s\n", msg->data);
}
