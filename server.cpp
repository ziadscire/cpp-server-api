/*
 * =============================================================
 *   Instruction Set Interpreter - HTTP API Server
 *   Compile: g++ server.cpp -o server -lmicrohttpd -std=c++17
 *   Run    : ./server
 *   Default port: 8080
 * =============================================================
 */

#include <microhttpd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <unordered_map>

using namespace std;

// =============================================================
//  CORE LOGIC (same as original, returns string instead of cout)
// =============================================================

string printDescription(int inst) {
    ostringstream oss;
    oss << "Result: 0x"
        << hex << uppercase
        << setw(4) << setfill('0')
        << inst << " -> ";

    switch (inst) {
        case 0x7800: oss << "CLA: Clear Accumulator";         return oss.str();
        case 0x7400: oss << "CLE: Clear E";                   return oss.str();
        case 0x7200: oss << "CMA: Complement Accumulator";    return oss.str();
        case 0x7100: oss << "CME: Complement E";              return oss.str();
        case 0x7080: oss << "CIR: Circulate Right AC and E";  return oss.str();
        case 0x7040: oss << "CIL: Circulate Left AC and E";   return oss.str();
        case 0x7020: oss << "INC: Increment Accumulator";     return oss.str();
        case 0x7010: oss << "SPA: Skip if AC is Positive";    return oss.str();
        case 0x7008: oss << "SNA: Skip if AC is Negative";    return oss.str();
        case 0x7004: oss << "SZA: Skip if AC is Zero";        return oss.str();
        case 0x7002: oss << "SZE: Skip if E is Zero";         return oss.str();
        case 0x7001: oss << "HLT: Halt Computer";             return oss.str();
        case 0xF800: oss << "INP: Input character to AC";     return oss.str();
        case 0xF400: oss << "OUT: Output character from AC";  return oss.str();
        case 0xF200: oss << "SKI: Skip on input flag";        return oss.str();
        case 0xF100: oss << "SKO: Skip on output flag";       return oss.str();
        case 0xF080: oss << "ION: Interrupt On";              return oss.str();
        case 0xF040: oss << "IOF: Interrupt Off";             return oss.str();
    }

    int opcode = (inst >> 12) & 0x7;
    int I      = (inst >> 15) & 0x1;
    int addr   = inst & 0x0FFF;
    string mode = (I == 1) ? "Indirect" : "Direct";
    string ops[] = {"AND","ADD","LDA","STA","BUN","BSA","ISZ"};

    if (opcode < 7)
        oss << ops[opcode] << " at 0x" << hex << addr << " (" << mode << ")";
    else
        oss << "Unknown Instruction";

    return oss.str();
}

string interpret(string code) {
    try {
        if      (code.length() == 16) return printDescription(stoi(code, nullptr, 2));
        else if (code.length() ==  4) return printDescription(stoi(code, nullptr, 16));
        else    return "ERROR: Invalid length. Expected 4-digit HEX or 16-bit Binary.";
    }
    catch (...) {
        return "ERROR: Could not parse the instruction.";
    }
}

// Simple JSON escape
string jsonEscape(const string& s) {
    string out;
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "";
        else                out += c;
    }
    return out;
}

// Parse a key from a simple JSON body: {"key":"value"}
string jsonGet(const string& body, const string& key) {
    string search = "\"" + key + "\"";
    size_t pos = body.find(search);
    if (pos == string::npos) return "";
    pos = body.find(':', pos);
    if (pos == string::npos) return "";
    pos = body.find('"', pos);
    if (pos == string::npos) return "";
    size_t start = pos + 1;
    size_t end = body.find('"', start);
    while (end != string::npos && body[end-1] == '\\') end = body.find('"', end+1);
    if (end == string::npos) return "";
    return body.substr(start, end - start);
}

// =============================================================
//  HTTP Handler data
// =============================================================
struct PostData {
    string data;
};

static MHD_Result postDataIterator(void* cls, enum MHD_ValueKind, const char*, const char*, const char*, const char*, const char* data, uint64_t, size_t size) {
    PostData* pd = (PostData*)cls;
    pd->data.append(data, size);
    return MHD_YES;
}

// =============================================================
//  Route: POST /api/part1  body: {"content":"7800\nF400\n..."}
// =============================================================
string handlePart1(const string& body) {
    string content = jsonGet(body, "content");
    if (content.empty()) return "{\"error\":\"No content provided\"}";

    // unescape \n
    string lines_raw;
    for (size_t i = 0; i < content.size(); i++) {
        if (content[i] == '\\' && i+1 < content.size() && content[i+1] == 'n') {
            lines_raw += '\n'; i++;
        } else lines_raw += content[i];
    }

    vector<string> results;
    istringstream ss(lines_raw);
    string line;
    while (getline(ss, line)) {
        if (line.empty()) continue;
        string code = line;
        for (auto& c : code) c = toupper(c);
        string res = interpret(code);
        string entry = "{\"input\":\"" + jsonEscape(code) + "\",\"result\":\"" + jsonEscape(res) + "\"}";
        results.push_back(entry);
    }

    string json = "{\"count\":" + to_string(results.size()) + ",\"results\":[";
    for (size_t i = 0; i < results.size(); i++) {
        json += results[i];
        if (i + 1 < results.size()) json += ",";
    }
    json += "]}";
    return json;
}

// =============================================================
//  Route: POST /api/part2  body: {"code":"7800"}
// =============================================================
string handlePart2(const string& body) {
    string code = jsonGet(body, "code");
    if (code.empty()) return "{\"error\":\"No code provided\"}";
    for (auto& c : code) c = toupper(c);
    string res = interpret(code);
    return "{\"input\":\"" + jsonEscape(code) + "\",\"result\":\"" + jsonEscape(res) + "\"}";
}

// =============================================================
//  Route: POST /api/part3
//  body: {"symbol":"ADD","address":"0A1","mode":"D"}
// =============================================================
string handlePart3(const string& body) {
    string symbol = jsonGet(body, "symbol");
    if (symbol.empty()) return "{\"error\":\"No symbol provided\"}";
    for (auto& c : symbol) c = toupper(c);

    unordered_map<string,int> regMap = {
        {"CLA",0x7800},{"CLE",0x7400},{"CMA",0x7200},{"CME",0x7100},
        {"CIR",0x7080},{"CIL",0x7040},{"INC",0x7020},{"SPA",0x7010},
        {"SNA",0x7008},{"SZA",0x7004},{"SZE",0x7002},{"HLT",0x7001},
        {"INP",0xF800},{"OUT",0xF400},{"SKI",0xF200},{"SKO",0xF100},
        {"ION",0xF080},{"IOF",0xF040}
    };
    unordered_map<string,int> memMap = {
        {"AND",0},{"ADD",1},{"LDA",2},{"STA",3},{"BUN",4},{"BSA",5},{"ISZ",6}
    };

    int inst_code = -1;

    if (regMap.count(symbol)) {
        inst_code = regMap[symbol];
    } else if (memMap.count(symbol)) {
        string addrHex = jsonGet(body, "address");
        string modeStr = jsonGet(body, "mode");
        if (addrHex.empty()) return "{\"error\":\"Address required for memory instruction\"}";
        for (auto& c : modeStr) c = toupper(c);
        int address, opcode = memMap[symbol];
        try { address = stoi(addrHex, nullptr, 16) & 0x0FFF; }
        catch (...) { return "{\"error\":\"Invalid hex address\"}"; }
        int addrMode = (modeStr == "I") ? 1 : 0;
        inst_code = (addrMode << 15) | (opcode << 12) | address;
    } else {
        return "{\"error\":\"Unknown symbol: " + symbol + "\"}";
    }

    char buffer[5];
    sprintf(buffer, "%04X", inst_code);
    string hex4(buffer);
    string res = interpret(hex4);
    return "{\"symbol\":\"" + symbol + "\",\"hex\":\"" + hex4 + "\",\"result\":\"" + jsonEscape(res) + "\"}";
}

// =============================================================
//  Main HTTP handler
// =============================================================
struct ConnectionData {
    PostData postData;
    struct MHD_PostProcessor* pp;
};

static MHD_Result handleRequest(void* cls, struct MHD_Connection* conn,
    const char* url, const char* method, const char*, const char* upload_data,
    size_t* upload_data_size, void** con_cls)
{
    // CORS preflight
    if (strcmp(method, "OPTIONS") == 0) {
        struct MHD_Response* resp = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(resp, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        MHD_add_response_header(resp, "Access-Control-Allow-Headers", "Content-Type");
        MHD_Result r = MHD_queue_response(conn, MHD_HTTP_OK, resp);
        MHD_destroy_response(resp);
        return r;
    }

    if (*con_cls == nullptr) {
        ConnectionData* cd = new ConnectionData();
        cd->pp = nullptr;
        *con_cls = cd;
        return MHD_YES;
    }

    ConnectionData* cd = (ConnectionData*)*con_cls;

    if (strcmp(method, "POST") == 0) {
        if (*upload_data_size > 0) {
            cd->postData.data.append(upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        }

        // Process request
        string responseBody;
        string urlStr(url);

        if      (urlStr == "/api/part1") responseBody = handlePart1(cd->postData.data);
        else if (urlStr == "/api/part2") responseBody = handlePart2(cd->postData.data);
        else if (urlStr == "/api/part3") responseBody = handlePart3(cd->postData.data);
        else responseBody = "{\"error\":\"Unknown endpoint\"}";

        struct MHD_Response* resp = MHD_create_response_from_buffer(
            responseBody.size(), (void*)responseBody.c_str(), MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(resp, "Content-Type", "application/json");
        MHD_add_response_header(resp, "Access-Control-Allow-Origin", "*");
        MHD_Result r = MHD_queue_response(conn, MHD_HTTP_OK, resp);
        MHD_destroy_response(resp);
        return r;
    }

    // 404
    string notFound = "{\"error\":\"Not found\"}";
    struct MHD_Response* resp = MHD_create_response_from_buffer(
        notFound.size(), (void*)notFound.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(resp, "Access-Control-Allow-Origin", "*");
    MHD_Result r = MHD_queue_response(conn, MHD_HTTP_NOT_FOUND, resp);
    MHD_destroy_response(resp);
    return r;
}

static void requestCompleted(void*, struct MHD_Connection*, void** con_cls, enum MHD_RequestTerminationCode) {
    ConnectionData* cd = (ConnectionData*)*con_cls;
    if (cd) { delete cd; *con_cls = nullptr; }
}

int main() {
    const int PORT = 8080;
    struct MHD_Daemon* daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD, PORT, nullptr, nullptr,
        &handleRequest, nullptr,
        MHD_OPTION_NOTIFY_COMPLETED, &requestCompleted, nullptr,
        MHD_OPTION_END
    );
    if (!daemon) { cerr << "Failed to start server.\n"; return 1; }
    cout << "========================================\n";
    cout << "  Instruction Set Interpreter - Server  \n";
    cout << "  Running on http://localhost:" << PORT << "\n";
    cout << "  Press ENTER to stop.\n";
    cout << "========================================\n";
    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}
