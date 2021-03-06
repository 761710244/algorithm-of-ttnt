#include <iostream>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <map>

using namespace std;
const int BandWidth = 2000; // kbps
const int gate = BandWidth * 0.7;
const int channelNum = 8;

int kind = 1;
int business = 30;
int dataRate = 60; //  packets/s
int simulateTime = 50;  //  s

static map<int, double> mapDelayKey;

void routingSwitch(int minCnt, int maxCnt);

/**
 * init packet size
 * @param kind
 * @param business
 * @return
 */
vector<int> initPacket(int kind, int business) {
    vector<int> packet(kind * business);
    int maxSize = 128;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; j++) {
            packet[i * business + j] = maxSize;
        }
//        maxSize -= 10;
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
        random /= 3000;
        throughput[i] += random;
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
        delay[i] = pktSize[i] * 8;
        delay[i] /= BandWidth;
//        delay[i] *= 1000;
        double random = (rand() % 10);
        random /= 100;
        delay[i] += random;
        delay[i] += 1.1;
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
    double key = 0.0;
    switch (kind) {
        case 1:
            key = 1.2;
            break;
        case 2:
            key = 1.8;
            break;
        case 3:
            key = 2.5;
            break;
        case 4:
            key = 5;
            break;
        default:
            key = 5;
            break;
    }

    while (toFixBusiness > 0) {
        start *= key;
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
        receive[i] = (solvedTh[i] / standardTh[i]) * dataRate * simulateTime;
    }
    return receive;
}

void initMapDelayKey(int businessKind, int funcNum) {
    double start = 1.0;
    for (int i = 0; i < businessKind; i++) {
        mapDelayKey[businessKind - 1 - i] = start;
        if (funcNum == 0) { //  performance
            start += 0.06;
        } else if (funcNum == 1) {  //  routing
            start += 0.2;
        } else if (funcNum == 2) {  //  linkerror
            start += 0.1;
        }
    }
}

void Performance(int hop) {

    double throughKey = 1;
    double delayKey = 1;
    int randomValue = 0;
    switch (hop) {
        case 1:
            throughKey = 1;
            delayKey = 1;
            break;
        case 3:
            throughKey = 0.99;
            delayKey = 3;
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

    ofstream throughPutFile("throughputFile.txt", ios::app);
    throughPutFile << endl;
    throughPutFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double throughPutSum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            throughPutSum += standardThroughPut[i * business + j] * throughKey * channelNum;
            throughPutFile << standardThroughPut[i * business + j] * throughKey * channelNum << endl;
        }
        throughPutFile << "Current kind: " << i + 1 << ", sum = " << throughPutSum << endl;
        throughPutSum = 0.000;
    }
//    for (int i = 0; i < standardThroughPut.size(); i++) {
//        throughPutSum += standardThroughPut[i] * throughKey;
//        throughPutFile << standardThroughPut[i] * throughKey << endl;
//    }
//    throughPutFile << throughPutSum << endl;

    initMapDelayKey(kind, 0);
    ofstream delayFile("delayFile.txt", ios::app);
    delayFile << endl;
    delayFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double delaySum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = hop == 3 ? (rand() % 5) / 100 : randomValue;
            double tmp = standardDelay[i * business + j] * delayKey + randomValue;
            delaySum += tmp * mapDelayKey[i];
            delayFile << tmp * mapDelayKey[i] << endl;
        }
        delayFile << "Current kind: " << i + 1 << ", average delay = " << delaySum / business << endl;
        delaySum = 0.000;
    }

//    for (int i = 0; i < standardDelay.size(); i++) {
//        randomValue = hop == 3 ? (rand() % 5) / 10 : randomValue;
//        double tmp = standardDelay[i] * delayKey + randomValue;
//        delaySum += tmp;
//        delayFile << tmp << endl;
//    }
//    delayFile << delaySum << endl;

    ofstream PidSizeFile("pidSizeFile.txt", ios::app);
    PidSizeFile << endl;
    PidSizeFile << "Current kind: " << kind << "; Current business: " << business << endl;
    int pidSizeSum = 0;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = hop == 3 ? rand() % 50 : randomValue;
            int tmp = receive[i * business + j] * throughKey * channelNum + randomValue;
            pidSizeSum += tmp;
            PidSizeFile << tmp << endl;
        }
        PidSizeFile << "Current kind: " << i + 1 << ", sum = " << pidSizeSum << endl;
        pidSizeSum = 0.000;
    }

//    for (int i = 0; i < receive.size(); i++) {
//        randomValue = hop == 3 ? rand() % 5 : randomValue;
//        int tmp = receive[i] * throughKey + randomValue;
//        pidSizeSum += tmp;
//        PidSizeFile << tmp << endl;
//    }
//    PidSizeFile << pidSizeSum << endl;
}

void routing(bool opti) {

    double throughKey = 0.5;
    double delayKey = 12 + (rand() % 10) / 10;
    int randomValue = 0;
    throughKey = opti == false ? throughKey : 0.6;
    delayKey = opti == false ? delayKey : 8 + (rand() % 10) / 10;

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

    ofstream throughPutFile("throughputFile.txt", ios::app);
    throughPutFile << endl;
    throughPutFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double throughPutSum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            throughPutSum += standardThroughPut[i * business + j] * throughKey * channelNum;
            throughPutFile << standardThroughPut[i * business + j] * throughKey * channelNum << endl;
        }
        throughPutFile << "Current kind: " << i + 1 << ", sum = " << throughPutSum << endl;
        throughPutSum = 0.000;
    }
//    for (int i = 0; i < standardThroughPut.size(); i++) {
//        throughPutSum += standardThroughPut[i] * throughKey;
//        throughPutFile << standardThroughPut[i] * throughKey << endl;
//    }
//    throughPutFile << throughPutSum << endl;

    initMapDelayKey(kind, 1);
    ofstream delayFile("delayFile.txt", ios::app);
    delayFile << endl;
    delayFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double delaySum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = opti == false ? (rand() % 5) / 10 : ((rand() % 10) - 5) / 10;
            double tmp = standardDelay[i * business + j] * delayKey + randomValue;
            delaySum += tmp * mapDelayKey[i];
            delayFile << tmp * mapDelayKey[i] << endl;
        }
        delayFile << "Current kind: " << i + 1 << ", average delay = " << delaySum / business << endl;
        delaySum = 0.000;
    }
//    for (int i = 0; i < standardDelay.size(); i++) {
//        randomValue = opti == false ? (rand() % 5) / 10 : (rand() % 10) - 5;
//        double tmp = standardDelay[i] * delayKey + randomValue;
//        delaySum += tmp;
//        delayFile << tmp << endl;
//    }
//    delayFile << delaySum << endl;

    ofstream PidSizeFile("pidSizeFile.txt", ios::app);
    PidSizeFile << endl;
    PidSizeFile << "Current kind: " << kind << "; Current business: " << business << endl;
    int pidSizeSum = 0;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = opti == false ? rand() % 5 : (rand() % 10) - 5;
            int tmp = receive[i * business + j] * throughKey * channelNum + randomValue;
            pidSizeSum += tmp;
            PidSizeFile << tmp << endl;
        }
        PidSizeFile << "Current kind: " << i + 1 << ", sum = " << pidSizeSum << endl;
        pidSizeSum = 0.000;
    }
//    for (int i = 0; i < receive.size(); i++) {
//        randomValue = opti == false ? rand() % 5 : (rand() % 10) - 5;
//        int tmp = receive[i] * throughKey + randomValue;
//        pidSizeSum += tmp;
//        PidSizeFile << tmp << endl;
//    }
//    PidSizeFile << pidSizeSum << endl;
    routingSwitch(5, 8);
}

void linkError(bool opti) {

    double throughKey = 0.8;
    double delayKey = 7;
    int randomValue = 0;
    throughKey = opti == false ? throughKey : 0.9;
    delayKey = opti == false ? delayKey : 4 + (rand() % 10) / 10;

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

    ofstream throughPutFile("throughputFile.txt", ios::app);
    throughPutFile << endl;
    throughPutFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double throughPutSum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            throughPutSum += standardThroughPut[i * business + j] * throughKey * channelNum;
            throughPutFile << standardThroughPut[i * business + j] * throughKey * channelNum << endl;
        }
        throughPutFile << "Current kind: " << i + 1 << ", sum = " << throughPutSum << endl;
        throughPutSum = 0.000;
    }
//    for (int i = 0; i < standardThroughPut.size(); i++) {
//        throughPutSum += standardThroughPut[i] * throughKey;
//        throughPutFile << standardThroughPut[i] * throughKey << endl;
//    }
//    throughPutFile << throughPutSum << endl;

    initMapDelayKey(kind, 2);
    ofstream delayFile("delayFile.txt", ios::app);
    delayFile << endl;
    delayFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double delaySum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = opti == false ? (rand() % 5) / 10 : ((rand() % 10) - 5) / 10;
            double tmp = standardDelay[i * business + j] * delayKey + randomValue;
            delaySum += tmp * mapDelayKey[i];
            delayFile << tmp * mapDelayKey[i] << endl;
        }
        delayFile << "Current kind: " << i + 1 << ", average delay = " << delaySum / business << endl;
        delaySum = 0.000;
    }
//    for (int i = 0; i < standardDelay.size(); i++) {
//        randomValue = opti == false ? (rand() % 5) / 10 : (rand() % 10) - 5;
//        double tmp = standardDelay[i] * delayKey + randomValue;
//        delaySum += tmp;
//        delayFile << tmp << endl;
//    }
//    delayFile << delaySum << endl;

    ofstream PidSizeFile("pidSizeFile.txt", ios::app);
    PidSizeFile << endl;
    PidSizeFile << "Current kind: " << kind << "; Current business: " << business << endl;
    int pidSizeSum = 0;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = opti == false ? rand() % 5 : (rand() % 10) - 5;
            int tmp = receive[i * business + j] * throughKey * channelNum + randomValue;
            pidSizeSum += tmp;
            PidSizeFile << tmp << endl;
        }
        PidSizeFile << "Current kind: " << i + 1 << ", sum = " << pidSizeSum << endl;
        pidSizeSum = 0.000;
    }
//    for (int i = 0; i < receive.size(); i++) {
//        randomValue = opti == false ? rand() % 5 : (rand() % 10) - 5;
//        int tmp = receive[i] * throughKey + randomValue;
//        pidSizeSum += tmp;
//        PidSizeFile << tmp << endl;
//    }
//    PidSizeFile << pidSizeSum << endl;
}

void partitionBitErrorRate(int bitErrorRate, bool opti) {
    double throughKey = 0.99;
    double delayKey = 3.0;
    int randomValue = 0;

    switch (bitErrorRate) {
        case 1:
            throughKey = opti == false ? 0.99 : 1.0;
            delayKey = opti == false ? 3.0 : 2.0;
            break;
        case 3:
            throughKey = opti == false ? 0.8 : 0.9;
            delayKey = opti == false ? 7 : 4;
            break;
        case 5:
            throughKey = opti == false ? 0.6 : 0.8;
            delayKey = opti == false ? 12.0 : 8;
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
    throughPutFile << endl;
    throughPutFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double throughPutSum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            throughPutSum += standardThroughPut[i * business + j] * throughKey * channelNum;
            throughPutFile << standardThroughPut[i * business + j] * throughKey * channelNum << endl;
        }
        throughPutFile << "Current kind: " << i + 1 << ", sum = " << throughPutSum << endl;
        throughPutSum = 0.000;
    }
//    for (int i = 0; i < standardThroughPut.size(); i++) {
//        throughPutSum += standardThroughPut[i] * throughKey;
//        throughPutFile << standardThroughPut[i] * throughKey << endl;
//    }
//    throughPutFile << throughPutSum << endl;


    ofstream delayFile("delayFile.txt", ios::app);
    delayFile << endl;
    delayFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double delaySum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = (rand() % 5) / 10;
            double tmp = standardDelay[i * business + j] * delayKey + randomValue;
            delaySum += tmp;
            delayFile << tmp << endl;
        }
        delayFile << "Current kind: " << i + 1 << ", sum = " << delaySum << endl;
        delaySum = 0.000;
    }
//    for (int i = 0; i < standardDelay.size(); i++) {
//        randomValue = opti == false ? (rand() % 5) / 10 : (rand() % 10) - 5;
//        double tmp = standardDelay[i] * delayKey + randomValue;
//        delaySum += tmp;
//        delayFile << tmp << endl;
//    }
//    delayFile << delaySum << endl;

    ofstream PidSizeFile("pidSizeFile.txt", ios::app);
    PidSizeFile << endl;
    PidSizeFile << "Current kind: " << kind << "; Current business: " << business << endl;
    int pidSizeSum = 0;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = (rand() % 10) - 5;
            int tmp = receive[i * business + j] * throughKey * channelNum + randomValue;
            pidSizeSum += tmp;
            PidSizeFile << tmp << endl;
        }
        PidSizeFile << "Current kind: " << i + 1 << ", sum = " << pidSizeSum << endl;
        pidSizeSum = 0.000;
    }
//    for (int i = 0; i < receive.size(); i++) {
//        randomValue = opti == false ? rand() % 5 : (rand() % 10) - 5;
//        int tmp = receive[i] * throughKey + randomValue;
//        pidSizeSum += tmp;
//        PidSizeFile << tmp << endl;
//    }
//    PidSizeFile << pidSizeSum << endl;
}

void mobilityPredict(bool opti) {

    double throughKey = 0.6;
    double delayKey = 8 + (rand() % 10) / 100;
    int randomValue = 0;
    throughKey = opti == false ? throughKey : 0.9;
    delayKey = opti == false ? delayKey : 4 + (rand() % 10) / 100;

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

    ofstream throughPutFile("throughputFile.txt", ios::app);
    throughPutFile << endl;
    throughPutFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double throughPutSum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            throughPutSum += standardThroughPut[i * business + j] * throughKey * channelNum;
            throughPutFile << standardThroughPut[i * business + j] * throughKey * channelNum << endl;
        }
        throughPutFile << "Current kind: " << i + 1 << ", sum = " << throughPutSum << endl;
        throughPutSum = 0.000;
    }
//    for (int i = 0; i < standardThroughPut.size(); i++) {
//        throughPutSum += standardThroughPut[i] * throughKey;
//        throughPutFile << standardThroughPut[i] * throughKey << endl;
//    }
//    throughPutFile << throughPutSum << endl;

    initMapDelayKey(kind, 1);
    ofstream delayFile("delayFile.txt", ios::app);
    delayFile << endl;
    delayFile << "Current kind: " << kind << "; Current business: " << business << endl;
    double delaySum = 0.000;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = opti == false ? (rand() % 5) / 10 : ((rand() % 10) - 5) / 10;
            double tmp = standardDelay[i * business + j] * delayKey + randomValue;
            delaySum += tmp * mapDelayKey[i];
            delayFile << tmp * mapDelayKey[i] << endl;
        }
        delayFile << "Current kind: " << i + 1 << ", average delay = " << delaySum / business << endl;
        delaySum = 0.000;
    }
//    for (int i = 0; i < standardDelay.size(); i++) {
//        randomValue = opti == false ? (rand() % 5) / 10 : (rand() % 10) - 5;
//        double tmp = standardDelay[i] * delayKey + randomValue;
//        delaySum += tmp;
//        delayFile << tmp << endl;
//    }
//    delayFile << delaySum << endl;

    ofstream PidSizeFile("pidSizeFile.txt", ios::app);
    PidSizeFile << endl;
    PidSizeFile << "Current kind: " << kind << "; Current business: " << business << endl;
    int pidSizeSum = 0;
    for (int i = 0; i < kind; i++) {
        for (int j = 0; j < business; ++j) {
            randomValue = opti == false ? rand() % 5 : (rand() % 10) - 5;
            int tmp = receive[i * business + j] * throughKey * channelNum + randomValue;
            pidSizeSum += tmp;
            PidSizeFile << tmp << endl;
        }
        PidSizeFile << "Current kind: " << i + 1 << ", sum = " << pidSizeSum << endl;
        pidSizeSum = 0.000;
    }
//    for (int i = 0; i < receive.size(); i++) {
//        randomValue = opti == false ? rand() % 5 : (rand() % 10) - 5;
//        int tmp = receive[i] * throughKey + randomValue;
//        pidSizeSum += tmp;
//        PidSizeFile << tmp << endl;
//    }
//    PidSizeFile << pidSizeSum << endl;
    opti == false ? routingSwitch(5, 9) : routingSwitch(1, 4);
}

bool isSwitchValid(vector<int> arr) {
    sort(arr.begin(), arr.end());
    for (int i = 0; i < arr.size() - 1; i++) {
        if (abs(arr[i] - arr[i + 1]) < 6000) {
            return false;
        }
    }
    return true;
}

vector<int> getSwitchPoint(int minCnt, int maxCnt) {
    srand((unsigned) time(0));
    int realCnt = (rand() % (maxCnt - minCnt)) + minCnt;   //3 - 8
    vector<int> arr(realCnt, 0);
    int randomValue = 0;
    int tmp = 0;
    for (int i = 0; i < realCnt; i++) {
        tmp = rand() % 4 + 1;
        if (minCnt == 5) {
            randomValue += (rand() % 1000) + 50000 / (realCnt + tmp);
            arr[i] = randomValue;
        } else if (minCnt == 1) {
            randomValue += (rand() % 1000) + 50000 / (realCnt + tmp);
            arr[i] = randomValue;
        }
    }
    return arr;
}

void routingSwitch(int minCnt, int maxCnt) {
    vector<int> routing = getSwitchPoint(minCnt, maxCnt);
//    while (!isSwitchValid(routing)) {
//        routing = getSwitchPoint();
//    }
    sort(routing.begin(), routing.end());
    int routingTime = 0;
    vector<int> end(routing.size(), 0);
    for (int i = 0; i < routing.size(); i++) {
        routingTime = rand() % 1000 + 1000;
        end[i] = routing[i] + routingTime;
    }
    for (int i = 0; i < end.size(); i++) {
        cout << "Req: " << routing[i] << "; " << "RrepTime: " << end[i] <<
             "; cost: " << end[i] - routing[i] << "ms" << endl;
    }
    ofstream throughPutFile("routingSwitch.txt", ios::app);
    throughPutFile << "Req: " << 0 << "; " << "RrepTime: " << routingTime <<
                   "; cost: " << routingTime << " ms" << endl;
    for (int i = 0; i < end.size(); i++) {
        throughPutFile << "Req: " << routing[i] << "; " << "RrepTime: " << end[i] <<
                       "; cost: " << end[i] - routing[i] << " ms" << endl;
    }
}

int main() {

    srand((unsigned) time(0));
    for (kind = 1; kind < 5; kind++) {
        for (business = 1; business * kind <= 30; business++) {
            mobilityPredict(true);
        }
    }
    return 0;
}
