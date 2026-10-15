#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "schema.pb.h"
#include "../include/json.hpp"

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

const char* SOCKET_PATH = "/tmp/rpc_study.sock";

int main() {
    const int WARMUP_RUNS = 1000;
    const int TEST_RUNS = 100;
    string payload_1kb(1024, 'a');

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    while (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        usleep(1000); // الانتظار لحين بدء الخادم
    }

    // 1. Warm-up
    char buf[4096];
    for (int i = 0; i < WARMUP_RUNS; ++i) {
        write(sock, payload_1kb.c_str(), payload_1kb.size());
        read(sock, buf, sizeof(buf));
    }

    // 2. القياس الفعلي
    ofstream csv_file("data/unix_socket_results.csv");
    csv_file << "run,total_rtt_ns,pb_serialize_ns,pb_deserialize_ns\n";

    rpc_study::EchoMessage pb_msg;
    pb_msg.set_payload(payload_1kb);

    for (int i = 0; i < TEST_RUNS; ++i) {
        // Serializing
        string pb_serialized;
        auto t1 = high_resolution_clock::now();
        pb_msg.SerializeToString(&pb_serialized);
        auto t2 = high_resolution_clock::now();
        double ser_ns = duration_cast<nanoseconds>(t2 - t1).count();

        // Round Trip Time (RTT) over Socket
        auto t3 = high_resolution_clock::now();
        write(sock, pb_serialized.c_str(), pb_serialized.size());
        ssize_t bytes_read = read(sock, buf, sizeof(buf));
        auto t4 = high_resolution_clock::now();
        double rtt_ns = duration_cast<nanoseconds>(t4 - t3).count();

        // Deserializing
        rpc_study::EchoMessage pb_out;
        auto t5 = high_resolution_clock::now();
        pb_out.ParseFromString(string(buf, bytes_read));
        auto t6 = high_resolution_clock::now();
        double deser_ns = duration_cast<nanoseconds>(t6 - t5).count();

        csv_file << i + 1 << "," << rtt_ns << "," << ser_ns << "," << deser_ns << "\n";
    }

    close(sock);
    csv_file.close();
    cout << "Unix Domain Socket measurements saved to data/unix_socket_results.csv" << endl;
    return 0;
}
