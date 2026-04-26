/*
 * =============================================================
 *   Project: Instruction Set Interpreter
 *   Course : Computer Architecture
 * =============================================================
 *
 *  Part 1 : Reading instructions from a text file
 *  Part 2 : Reading instructions from screen (Binary or HEX)
 *  Part 3 : Reading instruction as a symbol string (e.g. ADD, LDA)
 *
 * =============================================================
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <unordered_map>

using namespace std;


// =============================================================
//  SHARED FUNCTION: printDescription
//  Receives a 16-bit integer, returns its description as string.
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

    int opcode = (inst >> 12) & 0x7;   // bits 14-12
    int I      = (inst >> 15) & 0x1;   // bit  15  (0=Direct, 1=Indirect)
    int addr   = inst & 0x0FFF;         // bits 11-0

    string mode = (I == 1) ? "Indirect" : "Direct";

    switch (opcode) {
        case 0: oss << "AND at 0x" << hex << addr << " (" << mode << ")"; break;
        case 1: oss << "ADD at 0x" << hex << addr << " (" << mode << ")"; break;
        case 2: oss << "LDA at 0x" << hex << addr << " (" << mode << ")"; break;
        case 3: oss << "STA at 0x" << hex << addr << " (" << mode << ")"; break;
        case 4: oss << "BUN at 0x" << hex << addr << " (" << mode << ")"; break;
        case 5: oss << "BSA at 0x" << hex << addr << " (" << mode << ")"; break;
        case 6: oss << "ISZ at 0x" << hex << addr << " (" << mode << ")"; break;
        default: oss << "Unknown Instruction"; break;
    }

    return oss.str();
}


// =============================================================
//  SHARED FUNCTION: interpret
//  Receives a string (4-digit HEX or 16-bit Binary),
//  returns the decoded result as a string.
// =============================================================
string interpret(string code) {
    try {
        if      (code.length() == 16) return printDescription(stoi(code, nullptr, 2));
        else if (code.length() ==  4) return printDescription(stoi(code, nullptr, 16));
        else    return "Invalid input length. Expected 4-digit HEX or 16-bit Binary.";
    }
    catch (...) {
        return "Error: could not parse the instruction. Please check your input.";
    }
}


// =============================================================
//  SHARED FUNCTION: saveToFile
//  Asks the user if they want to save results to a text file.
//  If yes, asks for filename and writes all results to it.
// =============================================================
void saveToFile(const vector<string>& results) {

    cout << "\n----------------------------------------\n";
    cout << "  Do you want to save the output to a file?\n";
    cout << "  1 = Yes   |   0 = No\n";
    cout << ">> ";

    int choice;
    if (!(cin >> choice) || (choice != 1 && choice != 0)) {
        cin.clear();
        cin.ignore(100, '\n');
        cout << "Invalid input. Skipping save.\n";
        return;
    }

    if (choice == 0) {
        cout << "Output not saved.\n";
        return;
    }

    cout << "Enter output filename (e.g. output.txt): ";
    string outFileName;
    cin >> outFileName;

    ofstream outFile(outFileName);

    if (!outFile) {
        cout << "Error: could not create the file!\n";
        return;
    }

    outFile << "================================================\n";
    outFile << "   Instruction Set Interpreter - Output\n";
    outFile << "================================================\n\n";

    for (int i = 0; i < (int)results.size(); i++) {
        outFile << results[i] << "\n";
    }

    outFile << "\n================================================\n";
    outFile << "   Total instructions: " << results.size() << "\n";
    outFile << "================================================\n";

    outFile.close();

    cout << "Output saved successfully to: " << outFileName << "\n";
    cout << "----------------------------------------\n";
}


// =============================================================
//  PART 1: Reading Instructions from a Text File
// =============================================================
void part_1() {

    cout << "\n=== Part 1: Read Instructions from a File ===\n";
    cout << "Enter filename: ";

    string fileName;
    cin >> fileName;

    ifstream inputFile(fileName);

    if (!inputFile) {
        cout << "<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>\n";
        cout << "  File not found or cannot be opened!  \n";
        cout << "<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>\n";
        return;
    }

    vector<string> instructions;
    string currentLine;

    while (getline(inputFile, currentLine)) {
        if (!currentLine.empty()) {
            instructions.push_back(currentLine);
        }
    }

    inputFile.close();

    cout << "\n" << instructions.size()
         << " instruction(s) loaded successfully.\n";
    cout << "Interpreting...\n\n";

    vector<string> results;

    for (int i = 0; i < (int)instructions.size(); i++) {
        string line   = "Instruction " + to_string(i + 1) + ": " + instructions[i];
        string result = interpret(instructions[i]);

        cout << line   << "\n";
        cout << result << "\n\n";

        results.push_back(line);
        results.push_back(result);
        results.push_back("");
    }

    saveToFile(results);
}


// =============================================================
//  PART 2: Reading Instructions from Screen (Binary or HEX)
// =============================================================
void part_2() {

    cout << "\n=== Part 2: Enter Instructions from Screen (Binary or HEX) ===\n";
    cout << "Enter 4-digit HEX  (e.g. 7800)  or\n";
    cout << "      16-bit Binary (e.g. 0111100000000000)\n";
    cout << "Type EXIT to return to the menu.\n\n";

    vector<string> results;
    string InputCode;

    while (true) {
        cout << " >> ";
        cin >> InputCode;

        if (InputCode == "EXIT" || InputCode == "exit")
            break;

        if (InputCode.length() != 4 && InputCode.length() != 16) {
            cout << "Invalid input! Please enter exactly 4 HEX digits or 16 binary digits.\n";
            continue;
        }

        for (char& ch : InputCode) {
            ch = toupper(ch);
        }

        string line   = "Input: " + InputCode;
        string result = interpret(InputCode);

        cout << result << "\n\n";

        results.push_back(line);
        results.push_back(result);
        results.push_back("");
    }

    if (!results.empty()) {
        saveToFile(results);
    }
}


// =============================================================
//  PART 3: Reading Instruction as a Symbol String
//          (e.g. "ADD", "LDA", "HLT" ...)
// =============================================================
void part_3() {

    cout << "\n=== Part 3: Enter Instruction as a Symbol String ===\n";
    cout << "Type EXIT to return to the menu.\n\n";

    unordered_map<string, int> instruction_map = {
        {"CLA", 0x7800}, {"CLE", 0x7400}, {"CMA", 0x7200},
        {"CME", 0x7100}, {"CIR", 0x7080}, {"CIL", 0x7040},
        {"INC", 0x7020}, {"SPA", 0x7010}, {"SNA", 0x7008},
        {"SZA", 0x7004}, {"SZE", 0x7002}, {"HLT", 0x7001},
        {"INP", 0xF800}, {"OUT", 0xF400}, {"SKI", 0xF200},
        {"SKO", 0xF100}, {"ION", 0xF080}, {"IOF", 0xF040},
        {"AND", 0}, {"ADD", 1}, {"LDA", 2}, {"STA", 3},
        {"BUN", 4}, {"BSA", 5}, {"ISZ", 6}
    };

    vector<string> results;

    do {
        cout << ">> ";
        string symbol;
        cin >> symbol;

        for (int i = 0; i < (int)symbol.size(); i++) {
            symbol[i] = toupper(symbol[i]);
        }

        if (symbol == "EXIT")
            break;

        auto it = instruction_map.find(symbol);

        if (it == instruction_map.end()) {
            cout << "-----Invalid symbol, please try again-----\n";
            continue;
        }

        int value     = it->second;
        int inst_code = -1;

        if (value >= 0x7000) {
            inst_code = value;
        }
        else {
            int opcode = value;

            string addrHex;
            cout << "  Enter address in HEX (3 digits, e.g. 0A1): ";
            cin >> addrHex;

            int address;
            try {
                address = stoi(addrHex, nullptr, 16) & 0x0FFF;
            }
            catch (...) {
                cout << "  Invalid hex address. Please try again.\n";
                continue;
            }

            string i_check;
            cout << "  Direct or Indirect? (D/I): ";
            cin >> i_check;

            for (int i = 0; i < (int)i_check.size(); i++) {
                i_check[i] = toupper(i_check[i]);
            }

            int addrMode  = (i_check == "I") ? 1 : 0;
            inst_code = (addrMode << 15) | (opcode << 12) | address;
        }

        char buffer[5];
        sprintf(buffer, "%04X", inst_code);

        string code   = string(buffer);
        string line   = "Symbol: " + symbol + "  =>  HEX: " + code;
        string result = interpret(code);

        cout << result << "\n\n";

        results.push_back(line);
        results.push_back(result);
        results.push_back("");

    } while (true);

    if (!results.empty()) {
        saveToFile(results);
    }
}


// =============================================================
//  MAIN MENU
// =============================================================
int main() {

    cout << "========================================\n";
    cout << "     Instruction Set Interpreter        \n";
    cout << "     Computer Architecture Project      \n";
    cout << "========================================\n";

    int UserOption = -1;

    while (UserOption != 0) {

        cout << "\n1 ====> Read instructions from a text file\n";
        cout << "2 ====> Enter instruction from screen (Binary / HEX)\n";
        cout << "3 ====> Enter instruction as symbol string (e.g. ADD)\n";
        cout << "0 ====> Exit\n";
        cout << "Select Option: ";

        if (!(cin >> UserOption)) {
            cin.clear();
            cin.ignore(100, '\n');
            cout << "Invalid input, please enter a number.\n";
            continue;
        }

        if      (UserOption == 1) part_1();
        else if (UserOption == 2) part_2();
        else if (UserOption == 3) part_3();
        else if (UserOption == 0) cout << "\nGoodbye!\n";
        else                      cout << "Invalid option, please choose 0-3.\n";
    }

    return 0;
}
