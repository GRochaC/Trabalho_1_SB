#include "iostream"
#include "fstream"
#include "pre_processador.h"
#include "regex"
#include "string"

using namespace std;

map<string, pair<int,int>> opcode_table = {
    {"ADD"      ,{1, 2}},
    {"SUB"      ,{2, 2}},
    {"MULT"     ,{3, 2}},
    {"DIV"      ,{4, 2}},
    {"JMP"      ,{5, 2}},
    {"JMPN"     ,{6, 2}},
    {"JMPP"     ,{7, 2}},
    {"JMPZ"     ,{8, 2}},
    {"COPY"     ,{9, 3}},
    {"LOAD"     ,{10,2}},
    {"STORE"    ,{11,2}},
    {"INPUT"    ,{12,2}},
    {"OUTPUT"   ,{13,2}},
    {"STOP"     ,{14,1}}
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <file_path>\n";
        return 1;
    }

    string filePath = argv[1];
    ifstream inputFile(filePath);
    if (!inputFile) {
        cerr << "Error: Unable to open file " << filePath << "\n";
        return 1;
    }

    
    if (regex_match(filePath, regex(".*\\.asm$"))){
        return process(filePath);
    }

    if (regex_match(filePath, regex(".*\\.pre$"))){

        /*Primeira passagem*/
        string line;
        int line_counter = 1, address_counter = 0;
        map<string, int> label_table;
        while(getline(inputFile, line)){
            Instruction inst = parseInstruction(line);

            /*adiciona rotulo na TS*/
            if(inst.label != ""){
                if(label_table.count(inst.label)){
                    cerr << "Error in line " << line_counter
                         << ": Duplicate label '" << inst.label
                         << "'.\n";
                    return 1;
                }
                
                label_table[inst.label] = address_counter;
            }

            if(inst.label != "" and inst.operation == "") continue;
            
            if(opcode_table.count(inst.operation)) address_counter += opcode_table[inst.operation].second;

            else if(inst.operation == "CONST") address_counter++;

            else if(inst.operation == "SPACE") {
                if(inst.operand1 == "") address_counter++;
                else address_counter += stoi(inst.operand1);
            }
            else if(inst.operation == "SECTION") {
                line_counter++;
                continue;
            }
            else {
                cerr << "Error in line "  << line_counter
                     << ": Operation '" << inst.operation
                     << "' not identified.\n";

                return 1;
            }

            line_counter++;
        }

        // volta para o inicio do arquivo
        inputFile.clear();
        inputFile.seekg(0);
        
        /*Segunda passagem*/
        line_counter = 1;
        address_counter = 0;
        vector<string> code;
        while(getline(inputFile, line)){
            Instruction inst = parseInstruction(line);

            if(inst.operation == "SECTION") continue;
            
            if(inst.label != "" and inst.operation == "") continue;

            else if(inst.operation == "CONST"){
                if(inst.operand1 == ""){
                    cerr << "Error ";
                    return 1;
                }

                /*TODO: fazer um decodificador para hexadecimais */
                code.push_back(inst.operand1);
            }
            
            else if(inst.operation == "SPACE"){
                if(inst.operand1 == ""){
                    code.push_back("0");
                    address_counter++;
                } else {
                    
                    /*TODO: checar se o argumento eh valido */
                    for (int n = 0; n < stoi(inst.operand1); n++){
                        code.push_back("0");
                        address_counter++;
                    }
                }
            }

            else if(opcode_table.count(inst.operation)) {
                code.push_back(to_string(opcode_table[inst.operation].first));

                if(inst.operation == "COPY" and (inst.operand1 == "" or inst.operand2 == "")) {
                    cerr << "Error in line " << line_counter
                         << ": Copy operation expected 2 arguments.";
                    return 1;
                }

                else if(inst.operation == "STOP" and (inst.operand1 != "" or inst.operand2 != "")) {
                    cerr << "Error in line "<< line_counter 
                         << ": STOP operation expected 0 arguments.";
                    return 1;                }

                else if(inst.operation != "COPY" and inst.operand2 != ""){
                    cerr << "Error in line "<< line_counter 
                         << ": "<< inst.operation <<" operation expected 1 argument.";
                    return 1;
                }

                else {
                    if(label_table.count(inst.operand1)) code.push_back(to_string(label_table[inst.operand1]));
                    else if (inst.operand1 != "") code.push_back(inst.operand1);
                    
                    if(label_table.count(inst.operand2)) code.push_back(to_string(label_table[inst.operand2]));
                    else if(inst.operand2 != "") code.push_back(inst.operand2);
                }
            }

            else {
                cerr << "Error in line" << line_counter
                     << ": Operation '" << inst.operation
                     << "' not identified.\n";     
                     
                return 1;
            }
            line_counter++;
        }

        inputFile.close();

        string new_file_name = regex_replace(filePath, regex("\\.pre$"), ".obj");

        ofstream outputFile;
        outputFile.open(new_file_name);
        for(string address : code) outputFile << address << ' ';
        
        outputFile.close();

        return 0;
    }

    return 1;
}