#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "schema.pb.h"
#include "../include/json.hpp"

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

string echo_function(const string& input) {
    return input;
}

int main() {
    const int WARMUP_RUNS = 1000;
    const int TEST_RUNS = 100;
    
    string payload_1kb(1024, 'a');

    // 1. Warm-up
    for (int i = 0; i < WARMUP_RUNS; ++i) {
        volatile string res = echo_function(payload_1kb);
    }

    // فتح ملف لحفظ النتائج الخام
    ofstream csv_file("data/in_process_results.csv");
    csv_file << "run,direct_ns,pb_serialize_ns,pb_deserialize_ns,json_serialize_ns,json_deserialize_ns\n";

    // تجهيز كائنات Protobuf و JSON
    rpc_study::EchoMessage pb_msg;
    pb_msg.set_payload(payload_1kb);

    json json_msg;
    json_msg["payload"] = payload_1kb;

    for (int i = 0; i < TEST_RUNS; ++i) {
        // Direct Call
        auto t1 = high_resolution_clock::now();
        string res = echo_function(payload_1kb);
        auto t2 = high_resolution_clock::now();
        double direct_ns = duration_cast<nanoseconds>(t2 - t1).count();

        // Protobuf Serialize
        string pb_serialized;
        auto t3 = high_resolution_clock::now();
        pb_msg.SerializeToString(&pb_serialized);
        auto t4 = high_resolution_clock::now();
        double pb_ser_ns = duration_cast<nanoseconds>(t4 - t3).count();

        // Protobuf Deserialize
        rpc_study::EchoMessage pb_out;
        auto t5 = high_resolution_clock::now();
        pb_out.ParseFromString(pb_serialized);
        auto t6 = high_resolution_clock::now();
        double pb_deser_ns = duration_cast<nanoseconds>(t6 - t5).count();

        // JSON Serialize
        auto t7 = high_resolution_clock::now();
        string json_serialized = json_msg.dump();
        auto t8 = high_resolution_clock::now();
        double json_ser_ns = duration_cast<nanoseconds>(t8 - t7).count();

        // JSON Deserialize
        auto t9 = high_resolution_clock::now();
        json json_out = json::parse(json_serialized);
        auto t10 = high_resolution_clock::now();
        double json_deser_ns = duration_cast<nanoseconds>(t10 - t9).count();

        // كتابة السطر في CSV
        csv_file << i + 1 << "," << direct_ns << "," << pb_ser_ns << "," 
                 << pb_deser_ns << "," << json_ser_ns << "," << json_deser_ns << "\n";
    }

    csv_file.close();
    cout << "In-process measurements completed successfully! Raw data saved to data/in_process_results.csv" << endl;

    return 0;
}
