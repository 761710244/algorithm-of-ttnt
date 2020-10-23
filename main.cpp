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
vector<int> initPacket(int kind, int business) {
    vector<int> packet(kind * business);
    int maxSize = 500;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; j++) {
            packet[i * business + j] = maxSize;
        }
        maxSize -= 40;
    }
    return packet;
}

/**
 * get standard throughput
 * @param pkt_size
 * @param rate
 * @return
 */
vector<double> getStandardThroughPut(vector<int> pktSize, int rate) {
    vector<double> throughput(pktSize.size());
    for (int i = 0; i < pktSize.size(); i++) {
        throughput[i] = (pktSize[i] * 8 * rate) / 1000;
        double random = rand() % 500 + 500;
        throughput[i] -= 1 + random / 1000;
    }
    return throughput;
}

/**
 * select which to decrease
 * @param business
 * @param neeToChange
 * @return
 */
vector<int> initWhich(int business, int neeToChange) {
    vector<int> flag(business, 0);
    for (int i = 0; i < neeToChange; i++) {
        int randomValue = rand() % business;
        if (flag[randomValue] == 1) {
            i--;
        } else {
            flag[randomValue] = 1;
        }
    }
    return flag;
}

/**
 * adjust throughput
 * @param throughput
 * @param business
 * @return
 */
vector<double> solveThroughput(vector<double> throughput, int business) {
    double sum = 0.000;
    for (int i = 0; i < throughput.size(); i++) {
        sum += throughput[i];
    }
    if (sum < gate) {
        return throughput;
    }
    int randDecrease = rand() % 50;
    double distance = sum + randDecrease - gate;
    int needToFix = (rand() % business) + 3;
    double average = distance / (double) needToFix;
    vector<int> whichToChange = initWhich(business, needToFix);
    for (int i = 0; i < whichToChange.size(); i++) {
        if (whichToChange[i] == 1) {
            throughput[i] -= average;
        }
    }
    return throughput;
}

/**
 * get delay of each business
 * @param pkt_size
 * @return
 */
vector<double> getStandardDelay(vector<int> pktSize) {
    vector<double> delay(pktSize.size());
    for (int i = 0; i < pktSize.size(); i++) {
        delay[i] = (pktSize[i] + 104) * 8;
        delay[i] /= 1024;
        delay[i] /= BandWidth;
        delay[i] *= 1000;
        double random = rand() % 10;
        delay[i] += 0.4 + random / 100;
    }
    return delay;
}

/**
 * get when to fix delay
 * @param throughPut
 * @param kind
 * @param business
 * @return
 */
int getTopValue(vector<double> throughPut, int kind, int business) {
    double tmpTh = 0.000;
    vector<int> index(kind);
    for (int i = 0; i < kind; i++) {
        index[i] = i * business;
    }
    for (int i = 0; i < business; i++) {
        for (int j = 0; j < index.size(); j++) {
            tmpTh += throughPut[index[j]];
            index[j]++;
        }
        if (tmpTh > gate) {
            return (i + 1);
        }
    }
    return business;
}

vector<double> solveDelay(vector<double> delay, int business, int top) {
    if (business < top) {
        return delay;
    }
    int toFixBusiness = business - top + 1;
    int start = 2;
    while (toFixBusiness > 0) {
        start *= 2;
        toFixBusiness--;
    }
    return delay;
}

int main() {
    int kind = 3;
    int business = 10;
    int dataRate = 20; //  packets/s
    int nodeNum = kind * business * 2;

    //  get packet size of each business
    vector<int> packetSize = initPacket(kind, business);
    for (int i = 0; i < packetSize.size(); i++) {
        cout << packetSize[i] << " ";
    }
    cout << endl;

    //  get standard delay of each business
    vector<double> standardDelay = getStandardDelay(packetSize);
    for (int i = 0; i < standardDelay.size(); i++) {
        cout << standardDelay[i] << " ";
    }
    cout << endl;

    vector<double> standardThroughPut = getStandardThroughPut(packetSize, dataRate);
    for (int i = 0; i < standardThroughPut.size(); i++) {
        cout << standardThroughPut[i] << " ";
    }
    cout << endl;


    int top = getTopValue(standardThroughPut, kind, business);
    cout << top << endl;


    standardThroughPut = solveThroughput(standardThroughPut, business);
    for (int i = 0; i < standardThroughPut.size(); i++) {
        cout << standardThroughPut[i] << " ";
    }
    cout << endl;
    return 0;
}
