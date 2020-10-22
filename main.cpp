#include <iostream>
#include <vector>

using namespace std;

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

//vector<>
int main() {
    int kind = 2;
    int business = 2;
    vector<int> packet_size = init_packet(kind, business);
    for (int i = 0; i < packet_size.size(); i++) {
        cout << packet_size[i] << " ";
    }
    cout << endl;
    return 0;
}
