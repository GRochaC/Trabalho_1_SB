#pragma once

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

    Instruction(string line) {
        istringstream stream(line);

        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            label = line.substr(0, colonPos);
            stream.seekg(colonPos + 1);
        }

        stream >> operation;

        string operands;
        getline(stream, operands);
        istringstream operandStream(operands);

        getline(operandStream, operand1, ',');
        getline(operandStream, operand2, ',');

        auto trim = [](string& str) {
            str.erase(0, str.find_first_not_of(' '));
            str.erase(str.find_last_not_of(' ') + 1);
        };

        trim(operand1);
        trim(operand2);
    }

    Instruction(string line, map<string,string> equ_table) {
        istringstream stream(line);

        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            label = line.substr(0, colonPos);
            stream.seekg(colonPos + 1);
        }

        stream >> operation;

        string operands;
        getline(stream, operands);
        istringstream operandStream(operands);

        getline(operandStream, operand1, ',');
        getline(operandStream, operand2, ',');

        auto trim = [](string& str) {
            str.erase(0, str.find_first_not_of(' '));
            str.erase(str.find_last_not_of(' ') + 1);
        };

        trim(operand1);
        trim(operand2);

        if (equ_table.count(operand1)) {
            operand1 = equ_table[operand1];
        }
        
        if (equ_table.count(operand2)) {
            operand2 = equ_table[operand2];
        }
    }

    string format() {
        string ret = "";
        if(label != "") ret += label + ": ";
        ret += operation;
        if(operand1 != "") ret += " " + operand1;
        if(operand2 != "") ret += "," + operand2;

        return ret + "\n";
    }
};

int process(string file_name) {

    ifstream inputFile(file_name);

    string line;
    map<string, string> equ_table;  
    int line_counter = 0, section_data, section_text;
    bool flag_if = false;

    vector<Instruction> code;

    auto formatLine = [](string line) {
        line = regex_replace(line, regex(";.*|^\\s+|\\s+$"), "");
        line = regex_replace(line, regex("\\s+"), " ");

        transform(line.begin(), line.end(), line.begin(), [](unsigned char c) {
            return toupper(c);
        });

        return line;
    };

    while (getline(inputFile, line)) {
        line = formatLine(line);
        
        if(empty(line)) continue;

        if(flag_if) {
            flag_if = false;
            continue;
        }

        Instruction inst = Instruction(line, equ_table);

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

            else {
                cerr << "Semantic Error: Duplicate EQU definition.\n";
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
