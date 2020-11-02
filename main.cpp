#include <iostream>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <time.h>

using namespace std;
const int BandWidth = 2000; // kbps
const int gate = 1600;

int kind = 1;
int business = 30;
int dataRate = 20; //  packets/s

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
            packet[i * business + j] = maxSize - 48;
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
    if (neeToChange >= business) {
        vector<int> flag(business, 1);
        return flag;
    }
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
 * get which kind business all throughput
 * @param throughput
 * @param index
 * @param business
 * @return
 */
double getKindBusinessTh(vector<double> throughput, int kind, int business) {
    double sum = 0.000;
    int start = (kind - 1) * business;
    for (int i = start; i < start + business; i++) {
        sum += throughput[i];
    }
    return sum;
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
    int needToFix = (rand() % (business - 2)) + 3;
    int start = 1;
    double kindOne = getKindBusinessTh(throughput, start, business);
    //  keep high priority business to send
    while (kindOne < distance) {
        distance -= kindOne;
        start++;
        kindOne = getKindBusinessTh(throughput, start, business);
    }
    double average = distance / (double) needToFix;
    while (average > throughput[(start - 1) * business]) {
        needToFix = (rand() % (business - 2)) + 3;
        average = distance / (double) needToFix;
    }
    vector<int> whichToChange = initWhich(business, needToFix);
    for (int i = 0; i < (start - 1) * business; i++) {
        throughput[i] = 0;
    }
    for (int i = (start - 1) * business; i < (start - 1) * business + business; i++) {
        if (whichToChange[i - ((start - 1) * business)] == 1) {
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
    return 30;
}

/**
 * get yuzhi of delay
 * @param delay
 * @param top
 * @return
 */
double getDelayGate(vector<double> delay, int top) {
    double sum = 0.000;
    for (int i = 0; i < top; i++) {
        sum += delay[i];
    }
    return sum;
}

/**
 * compute the distance and random increase business
 * @param delay
 * @param business
 * @param top
 * @return
 */
vector<double> solveDelay(vector<double> delay, int business, int top) {
    if (business < top) {
        return delay;
    }
    int toFixBusiness = business - top + 1;
    double start = 1.1;
    while (toFixBusiness > 0) {
        start *= 1.2;
        toFixBusiness--;
    }
    double sum = getDelayGate(delay, top);
    double standard = sum * start;
    double distance = standard - sum;
    int needToFix = (rand() % (business - 2)) + 3;
    double average = distance / (double) needToFix;
    vector<int> whichToChange = initWhich(business, needToFix);
    for (int i = 0; i < whichToChange.size(); i++) {
        if (whichToChange[i] == 1) {
            delay[i] += average;
        }
    }
    return delay;
}

/**
 * get how many packets has received
 * @param standardTh
 * @param solvedTh
 * @return
 */
vector<int> getReceivePackets(vector<double> standardTh, vector<double> solvedTh) {
    vector<int> receive(standardTh.size());
    for (int i = 0; i < standardTh.size(); i++) {
        receive[i] = (solvedTh[i] / standardTh[i]) * 1000;
    }
    return receive;
}

void SinglePerformance(int hop) {

    double throughKey = 1;
    double delayKey = 1;
    int randomValue = 0;
    switch (hop) {
        case 1:
            throughKey = 1;
            delayKey = 1;
            break;
        case 3:
            throughKey = 0.95;
            delayKey = 3;
            randomValue = rand() % 10;
            break;
        default:
            break;
    }
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

    vector<double> tmpThroughPut = standardThroughPut;

    int top = getTopValue(standardThroughPut, kind, business);
    cout << top << endl;

    standardThroughPut = solveThroughput(standardThroughPut, business);
    for (int i = 0; i < standardThroughPut.size(); i++) {
        cout << standardThroughPut[i] << " ";
    }
    cout << endl;

    standardDelay = solveDelay(standardDelay, business, top);
    for (int i = 0; i < standardDelay.size(); i++) {
        cout << standardDelay[i] << " ";
    }
    cout << endl;

    vector<int> receive = getReceivePackets(tmpThroughPut, standardThroughPut);
    for (int i = 0; i < receive.size(); i++) {
        cout << receive[i] << " ";
    }
    cout << endl;

    ofstream throughPutFile("throughput.txt", ios::app);
    throughPutFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double throughPutSum = 0.000;
    for (int i = 0; i < standardThroughPut.size(); i++) {
        throughPutSum += standardThroughPut[i] * throughKey;
        throughPutFile << standardThroughPut[i] * throughKey << endl;
    }
    throughPutFile << throughPutSum << endl;


    ofstream delayFile("delayFile.txt", ios::app);
    delayFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double delaySum = 0.000;
    for (int i = 0; i < standardDelay.size(); i++) {
        double tmp = standardDelay[i] * delayKey + randomValue;
        delaySum += tmp;
        delayFile << tmp << endl;
    }
    delayFile << delaySum << endl;

    ofstream PidSizeFile("pidSizeFile.txt", ios::app);
    PidSizeFile << "Current kind: " << kind << "; Current business: " << business << endl;
    int pidSizeSum = 0;
    for (int i = 0; i < receive.size(); i++) {
        pidSizeSum += receive[i];
        PidSizeFile << receive[i] * throughKey << endl;
    }
    PidSizeFile << pidSizeSum << endl;
}

int main() {

    srand((unsigned) time(0));
    for (kind = 1; kind < 4; kind++) {
        for (business = 1; business * kind <= 30; business++) {
            SinglePerformance(3);
        }
    }
    return 0;
}
