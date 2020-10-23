#include <iostream>
#include <vector>
#include <stdlib.h>

using namespace std;
const int BandWidth = 2000; // kbps
const int gate = 1800;

/**
 * init packet size
 * @param kind
 * @param business
 * @return
 */
vector<int> init_packet(int kind, int business) {
    vector<int> packet(kind * business);
    int max_size = 500;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; j++) {
            packet[i * business + j] = max_size;
        }
        max_size -= 40;
    }
    return packet;
}

/**
 * get standard throughput
 * @param pkt_size
 * @param rate
 * @return
 */
vector<double> get_standard_th(vector<int> pkt_size, int rate) {
    vector<double> throughput(pkt_size.size());
    for (int i = 0; i < pkt_size.size(); i++) {
        throughput[i] = (pkt_size[i] * 8 * rate) / 1000;
        double random = rand() % 500 + 500;
        throughput[i] -= 1 + random / 1000;
    }
    return throughput;
}

vector<double> solve_throughput(vector<double> throughput) {

}

/**
 * get delay of each business
 * @param pkt_size
 * @return
 */
vector<double> get_standard_delay(vector<int> pkt_size) {
    vector<double> delay(pkt_size.size());
    for (int i = 0; i < pkt_size.size(); i++) {
        delay[i] = (pkt_size[i] + 104) * 8;
        delay[i] /= 1024;
        delay[i] /= BandWidth;
        delay[i] *= 1000;
        double random = rand() % 10;
        delay[i] += 0.4 + random / 100;
    }
    return delay;
}

int main() {
    int kind = 1;
    int business = 10;
    int data_rate = 20; //  packets/s
    int node_num = kind * business * 2;

    //  get packet size of each business
    vector<int> packet_size = init_packet(kind, business);
    for (int i = 0; i < packet_size.size(); i++) {
        cout << packet_size[i] << " ";
    }
    cout << endl;

    //  get standard delay of each business
    vector<double> standard_delay = get_standard_delay(packet_size);
    for (int i = 0; i < standard_delay.size(); i++) {
        cout << standard_delay[i] << " ";
    }
    cout << endl;

    vector<double> standard_th = get_standard_th(packet_size, data_rate);
    for (int i = 0; i < standard_th.size(); i++) {
        cout << standard_th[i] << " ";
    }
    cout << endl;
    return 0;
}
