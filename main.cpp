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
#include <algorithm>
#include <set>
#include <filesystem>
#include <thread>
#include <chrono>

void manejar_recordatorio(int segundos, const std::string &mensaje)
{
    std::this_thread::sleep_for(std::chrono::seconds(segundos));
    std::cout << "\nRecordatorio: " << mensaje << "\n"
              << std::endl;
}

// Función para agregar automáticamente comandos a favoritos
void agregar_a_favoritos(const std::string &comando, std::vector<std::string> &favoritos)
{
    if (comando.substr(0, 5) != "favs " && comando != "exit")
    {
        // Verificar que el comando no esté ya en la lista de favoritos
        if (std::find(favoritos.begin(), favoritos.end(), comando) == favoritos.end())
        {
            favoritos.push_back(comando);
        }
    }
}

int main()
{
    // Crear vector de comandos favoritos
    std::vector<std::string> favoritos;
    // PATH de archivo de favoritos, por defecto en la carpeta actual
    std::filesystem::path ARCHIVO_FAVORITOS = std::filesystem::current_path() / "misfavoritos.txt";
    while (true)
    {
        // Usamos readline para mostrar el prompt y leer el comando
        std::cout << "ShellT1: " << getcwd(NULL, 0);
        char *input = readline("$ ");
        if (!input)
        {
            continue;
        }

        std::string comando(input);

        // Liberamos la memoria asignada por readline
        free(input);

        // Agregamos el comando al historial
        if (!comando.empty())
        {
            add_history(comando.c_str());
        }

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

        //-------------------------------------------------------------------------------- COMANDO FAVS --------------------------------------------------------------------------------
        bool comando_interno = false;

        if (comandos[0][0] == "favs")
        {
            comando_interno = true;
            if (comandos[0].size() < 2)
            {
                std::cerr << "Error: Debe especificar un subcomando" << std::endl;
                continue;
            }
            // Crear archivo para comandos favoritos en ruta especificada
            if (comandos[0][1] == "crear")
            {
                if (comandos[0].size() < 3)
                {
                    std::cerr << "Error: Debe especificar la ruta del archivo" << std::endl;
                    continue;
                }
                // Si contiene un / se considera una ruta absoluta
                if (comandos[0][2].find("/") != std::string::npos)
                {
                    std::string path(getenv("HOME"));
                    // Si el / no esta al inicio se agrega
                    if (comandos[0][2][0] != '/')
                    {
                        std::string path(getenv("HOME"));
                        path += "/" + comandos[0][2];
                        ARCHIVO_FAVORITOS = path;
                    }
                    else
                    {
                        std::string path(getenv("HOME"));
                        path += comandos[0][2];
                        ARCHIVO_FAVORITOS = path;
                    }
                }
                // Si no contiene / se considera el directorio actual.
                else
                {
                    ARCHIVO_FAVORITOS = std::filesystem::current_path() / comandos[0][2];
                }

                // Se crea el archivo en el directorio especificado
                std::ofstream archivo(ARCHIVO_FAVORITOS);
                archivo.close();

                // Se verifica que se haya creado de manera correcta
                if (std::filesystem::exists(ARCHIVO_FAVORITOS))
                {
                    std::cout << "Archivo creado correctamente" << std::endl;
                    continue;
                }
                else
                {
                    std::cerr << "Error: No se pudo crear el archivo, se utilizara el directorio actual." << std::endl;
                    ARCHIVO_FAVORITOS = std::filesystem::current_path() / "misfavoritos.txt";
                    std::ofstream archivo(ARCHIVO_FAVORITOS);
                    archivo.close();
                    std::cout << "Archivo creado en: " << ARCHIVO_FAVORITOS << std::endl;
                    continue;
                }
            }

            // Mostrar lista de comandos favoritos
            else if (comandos[0][1] == "mostrar")
            {
                if (favoritos.empty())
                {
                    std::cout << "No hay comandos favoritos" << std::endl;
                    continue;
                }
                std::cout << "Comandos favoritos:" << std::endl;
                for (int i = 0; i < favoritos.size(); i++)
                {
                    std::cout << i + 1 << ". " << favoritos[i] << std::endl;
                }
            }

            // Eliminar comandos favoritos por número
            else if (comandos[0][1] == "eliminar")
            {
                // Parsear indices a eliminar
                std::vector<int> indices_a_eliminar;
                std::istringstream iss(comandos[0][2]);
                std::string numero;
                while (std::getline(iss, numero, ','))
                {
                    indices_a_eliminar.push_back(std::stoi(numero) - 1);
                }

                // Se ordenan en orden descendente
                std::sort(indices_a_eliminar.begin(), indices_a_eliminar.end(), std::greater<int>());

                std::vector<std::string> nuevos_comandos;

                // Eliminar comandos con indices especificados
                for (int i = favoritos.size() - 1; i >= 0; i--)
                {
                    for (int j = 0; j < indices_a_eliminar.size(); j++)
                    {
                        if (i == indices_a_eliminar[j])
                        {
                            favoritos.erase(favoritos.begin() + i);
                        }
                    }
                }
                std::cout << "Comandos eliminados" << std::endl;
            }
            // Buscar comandos favoritos
            else if (comandos[0][1] == "buscar")
            {
                for (int i = 0; i < favoritos.size(); i++)
                {
                    if (favoritos[i].find(comandos[0][2]) != std::string::npos)
                    {
                        std::cout << i + 1 << ". " << favoritos[i] << std::endl;
                    }
                }
            }
            // Borrar todos los comandos favoritos
            else if (comandos[0][1] == "borrar")
            {
                favoritos.clear();
                std::cout << "Todos los comandos favoritos han sido borrados." << std::endl;
            }
            // Ejecutar un comando favorito por su número
            else if (comandos[0][1] == "ejecutar")
            {
                // Verificar que el numero sea valido
                if (std::stoi(comandos[0][2]) < 1 || std::stoi(comandos[0][2]) > favoritos.size())
                {
                    std::cerr << "Error: Número de comando favorito inválido" << std::endl;
                    continue;
                }
                std::string comando_a_ejecutar = favoritos[std::stoi(comandos[0][2]) - 1];
                std::cout << "Ejecutando: " << comando_a_ejecutar << std::endl;
                system(comando_a_ejecutar.c_str());
            }
            // Cargar comandos favoritos desde el archivo
            else if (comandos[0][1] == "cargar")
            {
                // Se abre el archivo y se leen los comandos
                std::ifstream archivo(ARCHIVO_FAVORITOS);
                std::string linea;
                // Se limpia el vector de favoritos
                favoritos.clear();
                while (std::getline(archivo, linea))
                {
                    // Agregar los comandos del archivo de favoritos a el historial de la shell actual
                    favoritos.push_back(linea);
                }
                archivo.close();
                // Se despelga la lista de favoritos
                for (int i = 0; i < favoritos.size(); i++)
                {
                    std::cout << i + 1 << ". " << favoritos[i] << std::endl;
                }
            }
            // Guardar el historial de la sesión actual en los favoritos
            else if (comandos[0][1] == "guardar")
            {
                std::ofstream archivo(ARCHIVO_FAVORITOS);

                for (int i = 0; i < favoritos.size(); i++)
                {
                    if (std::find(favoritos.begin(), favoritos.begin() + i, favoritos[i]) == favoritos.begin() + i)
                    {
                        archivo << favoritos[i] << std::endl;
                    }
                }
                std::cout << "Historial guardado en " << ARCHIVO_FAVORITOS << std::endl;
            }
        }

        // ------------------------------ RECORDATORIO PERSONALIZADO -------------------------------------
        else if (comandos[0][0] == "set" && comandos[0][1] == "recordatorio")
        {
            comando_interno = true;
            if (comandos[0].size() < 4)
            {
                std::cerr << "Error: El comando debe incluir el tiempo y el mensaje" << std::endl;
                continue;
            }

            int segundos;
            try
            {
                segundos = std::stoi(comandos[0][2]);
            }
            catch (const std::invalid_argument &)
            {
                std::cerr << "Error: El tiempo debe ser un número válido" << std::endl;
                continue;
            }

            std::string mensaje;
            for (size_t i = 3; i < comandos[0].size(); ++i)
            {
                mensaje += comandos[0][i] + " ";
            }
            mensaje = mensaje.substr(0, mensaje.size() - 1); // Eliminar el espacio final

            // Crear un hilo para manejar el recordatorio
            std::thread recordatorio_thread(manejar_recordatorio, segundos, mensaje);
            recordatorio_thread.join(); // Desvincular el hilo para que continúe ejecutándose en segundo plano
            continue;
        }

        if (comando_interno)
        {
            continue;
        }

        agregar_a_favoritos(comando, favoritos);

        // ------------------------------ MANEJO DE PIPES -------------------------------------

        // Creamos un vector de pipes
        std::vector<int[2]> pipes(comandos.size() - 1);
        // Se crea un vector de procesos
        std::vector<pid_t> pids(comandos.size());

        // Crear todos los pipes necesarios
        for (int i = 0; i < comandos.size() - 1; i++)
        {
            if (pipe(pipes[i]) == -1)
            {
                std::cerr << "Error al crear el pipe" << std::endl;
                perror("pipe");
                exit(1);
            }
        }

        // Recorrer los comandos y crear procesos hijos
        for (int i = 0; i < comandos.size(); i++)
        {
            pids[i] = fork();

            if (pids[i] == -1)
            {
                std::cerr << "Error al crear el proceso" << std::endl;
                exit(1);
            }
            else if (pids[i] == 0)
            { // Proceso hijo
                if (i > 0)
                {                                        // No es el primer comando
                    dup2(pipes[i - 1][0], STDIN_FILENO); // Redirige stdin
                }
                if (i < comandos.size() - 1)
                {                                     // No es el ultimo comando
                    dup2(pipes[i][1], STDOUT_FILENO); // Redirige stdout
                }

                // Cerrar todos los pipes en el proceso hijo
                for (int j = 0; j < comandos.size() - 1; j++)
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // Preparar argumentos para execvp
                char **args = new char *[comandos[i].size() + 1];
                for (int j = 0; j < comandos[i].size(); j++)
                {
                    args[j] = const_cast<char *>(comandos[i][j].c_str());
                }
                args[comandos[i].size()] = NULL;

                execvp(args[0], args);

                // Si execvp falla
                std::cerr << "Error al ejecutar el comando" << std::endl;
                perror("execvp");
                delete[] args;
                exit(1);
            }
        }
        // Cerrar todos los pipes en el proceso padre
        for (int i = 0; i < comandos.size() - 1; i++)
        {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        // Esperar a que todos los procesos hijos terminen
        for (int i = 0; i < comandos.size(); i++)
        {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
    return 0;
}
