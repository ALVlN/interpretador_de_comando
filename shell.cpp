#include <iostream>      
#include <unistd.h>     
#include <sys/wait.h>    
#include <string>        
#include <vector>        
#include <sstream>       
#include <deque>         

using namespace std;

deque<string> historico;

// Função para adicionar e manter os últimos 10 comandos no histórico
void add_historico(const string& cmd_line) {
    historico.push_front(cmd_line); 
    if (historico.size() > 10) {
        historico.pop_back(); 
    }
}

void process_command(string command_line) {
    if (command_line.empty()) return;

    // Parte 1 (b): Processar a linha para separar comando e argumentos usando espaço
    vector<string> args;
    string word;
    stringstream ss(command_line);
    while (ss >> word) {
        args.push_back(word);
    }
    
    if (args.empty()) return;
    
    string command = args[0];

    // Parte 2: Comandos internos    
    if (command == "exit") {
        int status = 0;
        if (args.size() > 1) status = stoi(args[1]);
        exit(status); 
    }
    else if (command == "pwd") {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            cout << cwd << endl;
        }
    }
    else if (command == "cd") {
        if (args.size() < 2) {
            cout << "cd: argumento esperado" << endl;
        } else if (chdir(args[1].c_str()) != 0) {
            cout << "cd: diretorio nao existe" << endl;
        }
    }
    else if (command == "history") {
        if (args.size() == 1) {
            for (size_t i = 0; i < historico.size(); ++i) {
                cout << i << " " << historico[i] << endl;
            }
        } else if (args.size() == 2 && args[1] == "-c") {
            historico.clear();
        } else if (args.size() == 2) {
            try {
                int offset = stoi(args[1]);
                if (offset >= 0 && offset < historico.size()) {
                    string past_cmd = historico[offset];
                    cout << past_cmd << endl;
                    process_command(past_cmd);
                } else {
                    cout << "Erro: numero nao e valido" << endl;
                }
            } catch (...) {
                cout << "Erro no offset" << endl;
            }
        }
    }

    // Parte 3: Comandos externos
    else {
        vector<string> path_list = {"/bin", "/usr/bin", "/usr/local/bin"};
        
        string absolute_path = "";
        bool command_exists = false;

        // Procura o comando na lista de diretórios para formar o caminho absoluto
        for (const string& dir : path_list) {
            string test_path = dir + "/" + command;
            
            // Verifica se comando existe
            if (access(test_path.c_str(), F_OK) == 0) { 
                absolute_path = test_path;
                command_exists = true;
                break;
            }
        }

        if (command_exists) { 
            // Arquivo existe, agora verifica se é executável
            if (access(absolute_path.c_str(), X_OK) == 0) { 
                
                pid_t pid = fork();
                
                if (pid < 0) { 
                    cout << "Erro de execução!" << endl;
                    return;
                } 
                else if (pid == 0) {
                    
                    vector<char*> exec_argv;
                    for (auto& a : args) {
                        exec_argv.push_back(const_cast<char*>(a.c_str()));
                    }
                    exec_argv.push_back(nullptr);
                    
                    execve(absolute_path.c_str(), exec_argv.data(), NULL);
                    
                } 
                else {
                    waitpid(pid, nullptr, 0);
                }
            } else {
                cout << "permission denied: " << command << endl;
            }
        } else { 
            cout << "Command not found: " << command << endl;
        }
    }
}

int main() {
    while (true) {
        cout << "$"; 
        
        string command_line;
        if (!getline(cin, command_line)) {
            break;
        }
        if (!command_line.empty() && command_line.rfind("history", 0) != 0) {
            add_historico(command_line);
        }

        process_command(command_line);
    }
    return 0;
}