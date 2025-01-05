#include "pre_processador.h"

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

    if (regex_match(filePath, regex(".*\\.asm$"))) return process(filePath);

    if (regex_match(filePath, regex(".*\\.pre$"))){

        /*Primeira passagem*/
        string line;
        int line_counter = 1, address_counter = 0;
        map<string, int> label_table, definition_table;
        map<string, vector<int>> usage_table;
        bool looking_for_end = false;
        while(getline(inputFile, line)){
            Instruction inst = Instruction(line);

            /*adiciona rotulo na TS*/
            if(inst.label != ""){
                // adiciona o label na tabela de labels
                if(label_table.count(inst.label)){
                    cerr << "Semantic Error in line " << line_counter
                         << ": Duplicate label '" << inst.label
                         << "'.\n";
                    return 1;
                }
                
                //testa erro lexico do rotulo
                if(regex_search(inst.label, regex("^\\d|[^a-zA-Z0-9_]"))){
                    cerr << "Lexical Error in line " << line_counter <<  ": Wrong label name construction.\n";
                    return 1;
                }

                label_table[inst.label] = address_counter;

                // adiciona o label na tabela de definicao
                if(definition_table.count(inst.label)) definition_table[inst.label] = address_counter;
            }

            if(inst.label != "" and inst.operation == "") continue;
            
            if(usage_table.count(inst.operand1)) usage_table[inst.operand1].push_back(address_counter + 1);
            
            if(usage_table.count(inst.operand2)) usage_table[inst.operand2].push_back(address_counter + 2);

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
            
            else if(inst.operation == "BEGIN") {
                if(looking_for_end) {
                    cerr << "Syntax Error in line " << line_counter
                         << ": Cannot define nested modules.\n";

                    return 1;
                }
                
                looking_for_end = true;    
            }
            
            else if(inst.operation == "END") looking_for_end = false;

            else if(inst.operation == "EXTERN") {
                // adiciona na tabela de uso
                usage_table[inst.label] = {};
            }

            else if(inst.operation == "PUBLIC") {
                if(not looking_for_end){
                    cerr << "Syntax Error in line " << line_counter
                         << ": Module not defined.\n";
                    return 1;
                }
                // adiciona na tabela de definicao
                definition_table[inst.operand1] = 0;
            }

            else {
                cerr << "Syntax Error in line "  << line_counter
                     << ": Operation '" << inst.operation
                     << "' not identified.\n";

                return 1;
            }

            line_counter++;
        }

        // checa se o BEGIN foi finalizado
        if(looking_for_end) {
            cerr << "Syntax Error: Module without an end."; 
            return 1;
        }

        // volta para o inicio do arquivo
        inputFile.clear();
        inputFile.seekg(0);
        
        auto parseHex2Int = [](string str_hex) {
            if(regex_match(str_hex, regex("^0[xX].*"))) {
                unsigned int tmp_int = stoul(str_hex,nullptr,0);
                
                int num_int = static_cast<int16_t>(tmp_int);

                return to_string(num_int);
            } 
            
            else return str_hex;
        };

        /*Segunda passagem*/
        line_counter = 1;
        address_counter = 0;
        vector<string> obj_code, bitmap;
        while(getline(inputFile, line)){
            Instruction inst = Instruction(line);

            if(inst.operation == "SECTION" or inst.operation == "BEGIN" or inst.operation == "END" or inst.operation == "EXTERN" or inst.operation == "PUBLIC") continue;
            
            if(inst.label != "" and inst.operation == "") continue;

            else if(inst.operation == "CONST"){
                if(inst.operand1 == ""){
                    cerr << "Syntax Error in line " << line_counter << ": constant value not defined.";
                    return 1;
                }

                bitmap.push_back("0");

                obj_code.push_back(parseHex2Int(inst.operand1));
            }
            
            else if(inst.operation == "SPACE"){
                if(inst.operand1 == ""){
                    bitmap.push_back("0");
                    obj_code.push_back("0");
                    address_counter++;

                } else {
                    for (int n = 0; n < stoi(inst.operand1); n++){
                        bitmap.push_back("0");
                        obj_code.push_back("0");
                        address_counter++;
                    }
                }
            }

            else if(opcode_table.count(inst.operation)) {
                obj_code.push_back(to_string(opcode_table[inst.operation].first));

                if(inst.operation == "COPY" and (inst.operand1 == "" or inst.operand2 == "")) {
                    cerr << "Syntax Error in line " << line_counter
                         << ": Copy operation expected 2 arguments.";
                    return 1;
                }

                else if(inst.operation == "STOP" and (inst.operand1 != "" or inst.operand2 != "")) {
                    cerr << "Syntax Error in line "<< line_counter 
                         << ": STOP operation expected 0 arguments.";
                    return 1;                }

                else if(inst.operation != "COPY" and inst.operand2 != ""){
                    cerr << "Syntax Error in line "<< line_counter 
                         << ": "<< inst.operation <<" operation expected 1 argument.";
                    return 1;
                }
                else {
                    if(label_table.count(inst.operand1)) obj_code.push_back(to_string(label_table[inst.operand1]));
                    else if(usage_table.count(inst.operand1)) obj_code.push_back("0");
                    else if(inst.operand1 != ""){
                        cerr << "Semantic Error in line " << line_counter << ": label \"" << inst.operand1 << "\" not defined.";
                        return 1;
                    }
                    
                    if(label_table.count(inst.operand2)) obj_code.push_back(to_string(label_table[inst.operand2]));
                    else if(usage_table.count(inst.operand2)) obj_code.push_back("0");                
                    else if(inst.operand2 != ""){
                        cerr << "Semantic Error in line " << line_counter << ": label \"" << inst.operand2 << "\" not defined.";
                        return 1;
                    }
                }
           
                switch (opcode_table[inst.operation].second) {
                    case 1:
                        bitmap.push_back("0");
                        break;
                    case 2:
                        bitmap.push_back("0");
                        bitmap.push_back("1");
                        break;
                    case 3:
                        bitmap.push_back("0");
                        bitmap.push_back("1");
                        bitmap.push_back("1");
                        break;
                }
           }

            else {
                cerr << "Syntax Error in line " << line_counter
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

        if(not definition_table.empty()) for(auto [label,address] : definition_table) outputFile << "D, " << label << " " << address << '\n'; 
        
        if(not usage_table.empty()) {
            for(auto [label,addresses] : usage_table) 
                for(int address : addresses) outputFile << "U, " << label << " " << address << '\n';
        }
        
        if(not bitmap.empty() and (not definition_table.empty() or not usage_table.empty())) {
            outputFile << "R, ";
            for(string bit : bitmap) outputFile << bit << " ";
            outputFile << '\n';
        }

        for(string address : obj_code) outputFile << address << ' ';
        
        outputFile.close();

        return 0;
    }

    return 1;
}