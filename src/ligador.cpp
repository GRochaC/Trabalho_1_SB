#include "iostream"
#include "fstream"
#include "pre_processador.h"
#include "regex"
#include "string"

struct obj_code {
    map<string,int> definition_table;
    map<string,vector<int>> usage_table;
    vector<int> bitmap;
    vector<int> code;

    obj_code(ifstream& file) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string prefix;
            ss >> prefix; 

            if (prefix == "D,") {
                string key;
                int value;
                ss >> key >> value;
                definition_table[key] = value; 
            } else if (prefix == "U,") {
                string key;
                int value;
                ss >> key >> value;
                usage_table[key].push_back(value);    
            } else if (prefix == "R,") {
                int value;
                while (ss >> value) {
                    bitmap.push_back(value); 
                }
            } else {
                int value;
                code.push_back(stoi(prefix));
                while (ss >> value) {
                    code.push_back(value);
                }
            }
        }

        file.close();
    }

    string format() {
        string ret = "OBJECT CODE: ";
        for(auto [l,a] : definition_table) ret += "D, " + l + " " + to_string(a) + "\n"; 
        for(auto [l,as] : usage_table)
            for(auto a : as) ret += "U, " + l + " "+to_string(a)+"\n";
        for(auto b : bitmap) ret += "R, " + to_string(b) + " ";
        return ret; 
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <file_path> <file_path>\n";
        return 1;
    }

    string filePath_1 = argv[1], filePath_2 = argv[2];

    ifstream inputFile_1(filePath_1);
    if (!inputFile_1) {
        cerr << "Error: Unable to open file " << filePath_1 << "\n";
        return 1;
    }

    obj_code obj1 = obj_code(inputFile_1);
    
    ifstream inputFile_2(filePath_2);
    if (!inputFile_2) {
        cerr << "Error: Unable to open file " << filePath_2 << "\n";
        return 1;
    }

    obj_code obj2 = obj_code(inputFile_2);


    int correction_factor = (int) obj1.code.size();
    map<string, int> global_definition_table = obj1.definition_table;
    for(auto [l, a] : obj2.definition_table) {
        if(global_definition_table.count(l)){
            cerr << "Error: Found duplicate label " << l << " while linking.";
            return 1;
        }
        
        global_definition_table[l] = a + correction_factor;
    }

    for(auto [s, v] : obj1.usage_table){
        for(auto a : v){
            obj1.code[a] = global_definition_table[s];
        }
    }
    for(auto [s, v] : obj2.usage_table){
        for(auto a : v){
            obj2.code[a] = global_definition_table[s];
        }
    }
    
    string new_file_name = regex_replace(filePath_1, regex("\\.obj$"), ".e");

    ofstream outputFile(new_file_name);
    for(int a : obj1.code) outputFile << a << ' ';
    for(int a : obj2.code) outputFile << a << ' ';

    outputFile.close();

    return 0;
}