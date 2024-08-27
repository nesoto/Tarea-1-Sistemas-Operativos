#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

int main()
{
    while (true)
    {
        // Usamos readline para mostrar el prompt y leer el comando
        char *input = readline("Ingrese un comando: ");
        if (!input)
        {
            continue;
        }

        std::string comando(input);

        // Agregamos el comando al historial
        if (!comando.empty())
        {
            add_history(comando.c_str());
        }

        // Liberamos la memoria asignada por readline
        free(input);

        // Si se presiona enter y no hay comando, continua
        if (comando.empty())
        {
            continue;
        }
        // Si se escribe exit, termina el programa
        if (comando == "exit")
        {
            break;
        }

        // Se crea un vector de vectores de strings para almacenar los comandos
        std::vector<std::vector<std::string>> comandos;

        // Se crea un flujo de comandos
        std::istringstream ss(comando);

        std::vector<std::string> comando_actual;
        std::string token;
        // Se lee el comando y se separa por el pipe
        while (ss >> token)
        {
            if (token == "|")
            {
                comandos.push_back(comando_actual);
                comando_actual.clear();
            }
            else
            {
                comando_actual.push_back(token);
            }
        }
        comandos.push_back(comando_actual);

        // Creamos un vector de pipes
        std::vector<int[2]> pipes(comandos.size() - 1);
        // Se crea un vector de procesos
        std::vector<pid_t> pids(comandos.size());

        // Se crea un ciclo para recorrer los comandos
        for (int i = 0; i < comandos.size(); i++)
        {
            // Se crea un proceso
            pids[i] = fork();
            // Si es el proceso hijo
            if (pids[i] == -1)
            {
                std::cerr << "Error al crear el proceso" << std::endl;
                exit(1);
            }
            else if (pids[i] == 0)
            {
                if (i > 0)
                {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                    close(pipes[i - 1][1]);
                }
                if (i < comandos.size() - 1)
                {
                    dup2(pipes[i][1], STDOUT_FILENO);
                    close(pipes[i][0]);
                }

                // Ejecutamos los comandos guardados
                char **args = new char *[comandos[i].size() + 1];
                for (int j = 0; j < comandos[i].size(); j++)
                {
                    args[j] = const_cast<char *>(comandos[i][j].c_str());
                }
                args[comandos[i].size()] = NULL;
                execvp(args[0], args);
                // Si hay error
                std::cerr << "Error al ejecutar el comando" << std::endl;
                perror("execvp");
                delete[] args;
                exit(1);
            }
            if (i > 0)
            {
                close(pipes[i - 1][0]);
            }
            if (i < comandos.size() - 1)
            {
                if (pipe(pipes[i]) == -1)
                {
                    std::cerr << "Error al crear el pipe" << std::endl;
                    perror("pipe");
                    continue;
                }
                close(pipes[i][1]);
            }
        }
        // Esperar a que los procesos hijos terminen
        for (int i = 0; i < comandos.size(); i++)
        {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
    return 0;
}
