//
// Created by Alex on 2021/11/30.
//

#include "neighbor_table.h"
//by cyo
float getE(int A_send, int B_send, int A_recieved, int B_recieved) {
    return (float) (A_send * B_send) / (float) (A_recieved * B_recieved);
}

float getG(const int historyStab[], int neighbor_sum, int neighbor_change) {
    float result = 0;
    neighbor_change = (neighbor_change > 0) ? neighbor_change : -neighbor_change;
    for (int i = 0; i < NUM_STATES; i++) {
        if (this_host.node_stability[i] == 0 && historyStab[i] == 0) {
            result += K00 * (float) (neighbor_sum - neighbor_change) / (float) neighbor_sum;
        } else if (this_host.node_stability[i] == 1 && historyStab[i] == 1) {
            result += K11 * (float) (neighbor_sum - neighbor_change) / (float) neighbor_sum;
        } else {
            result += K01 * (float) (neighbor_sum - neighbor_change) / (float) neighbor_sum;
        }
    }
    return (result > 0) ? result : 0;
}


float updateCost(int E, int F, int G) {
    float result = A1 * E + B1 * F + C1 * G - (A1 + B1) * E * F - (A1 + C1) * E * G - (B1 + C1) * F * G +
                   (A1 + B1 + C1) * E * F * G;
    hello_send_clear();
    hello_received_clear();
    return result;
}
//cyo_end