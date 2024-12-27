#include <iostream>
#include <regex>
#include <fstream>
#include <string>
#include <map>
using namespace std;

struct Instruction {
    string label;
    string operation;
    string operand1;
    string operand2;

    string format() {
        string ret = "";
        if(label != "") ret += label + ": ";
        ret += operation;
        if(operand1 != "") ret += " " + operand1;
        if(operand2 != "") ret += "," + operand2;

        return ret + "\n";
    }
};

Instruction parseInstruction(string line, map<string,string>equ_table = {}) {
    Instruction instruction;

    istringstream stream(line);

    size_t colonPos = line.find(':');
    if (colonPos != string::npos) {
        instruction.label = line.substr(0, colonPos);
        stream.seekg(colonPos + 1);
    }

    stream >> instruction.operation;

    string operands;
    getline(stream, operands);
    istringstream operandStream(operands);

    getline(operandStream, instruction.operand1, ',');
    getline(operandStream, instruction.operand2, ',');

    auto trim = [](string& str) {
        str.erase(0, str.find_first_not_of(' '));
        str.erase(str.find_last_not_of(' ') + 1);
    };

    trim(instruction.operand1);
    trim(instruction.operand2);

    if (equ_table.count(instruction.operand1)) {
        instruction.operand1 = equ_table[instruction.operand1];
    }
    
    if (equ_table.count(instruction.operand2)) {
        instruction.operand2 = equ_table[instruction.operand2];
    }


    return instruction; 
}

string formatLine(string line) {
    /* remove comments and unecessary spacing */
    line = regex_replace(line, regex(";.*|^\\s+|\\s+$"), "");
    line = regex_replace(line, regex("\\s+"), " ");

    transform(line.begin(), line.end(), line.begin(), [](unsigned char c) {
        return toupper(c);
    });

    return line;
}


int process(string file_name) {

    ifstream inputFile(file_name);

    string line;
    map<string, string> equ_table;  
    int line_counter = 0, section_data, section_text;
    bool flag_if = false;

    vector<Instruction> code;

    while (getline(inputFile, line)) {
        line = formatLine(line);
        
        if(empty(line)) continue;

        if(flag_if) {
            flag_if = false;
            continue;
        }

        Instruction inst = parseInstruction(line, equ_table);

        if(inst.operation == "SECTION" and inst.operand1 == "DATA"){
            section_data = line_counter;
        }
        if(inst.operation == "SECTION" and inst.operand1 == "TEXT"){
            section_text = line_counter;
        }

        if(inst.operation == "EQU"){
            if(equ_table.find(inst.label) == equ_table.end()){
                equ_table[inst.label] = inst.operand1;
                continue;
            }
            else{
                cerr << "Error: Duplicate EQU definition.\n";
                return 1;
            }
        }
        if(inst.operation == "IF"){
            flag_if = (inst.operand1 == "0");
            continue;
        }

        code.push_back(inst);
        line_counter++;
    }
    
    inputFile.close();
    

    string new_file_name = regex_replace(file_name, regex("\\.asm$"), ".pre");
    
    //TODO: fazer a construcao do arquive em um loop
    ofstream outputFile;
    outputFile.open(new_file_name);
    for(int idx = section_text; idx < line_counter and idx != section_data; idx++) {
        Instruction inst = code[idx];
        outputFile << inst.format();
    }

    for(int idx = section_data; idx < line_counter and idx != section_text; idx++) {
        Instruction inst = code[idx];
        outputFile << inst.format();
    }

    outputFile.close();
    
    return 0;
}

/*
 LABEL: SPACE -> ocupa 1
 LABEL: SPACE 1 -> 1
 LABEL: SPACE 4 -> 4
*/