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


//Eliminar el hardcode para que pueda el usuario elegir el path
//Hacer que se pueda manejar carpetas con '' o con ""
//Cuando termine el recordatorio no se pegue la shell
const std::filesystem::path ARCHIVO_FAVORITOS = "misfavoritos.txt";

void manejar_recordatorio(int segundos, const std::string& mensaje) 
{
    std::this_thread::sleep_for(std::chrono::seconds(segundos));
    std::cout << "\nRecordatorio: " << mensaje <<"\n"<< std::endl;
}

// Función para agregar automáticamente comandos a favoritos
void agregar_a_favoritos(const std::string& comando) 
{
    if (comando.substr(0, 5) != "favs " && comando != "exit")
    {
        std::ofstream archivo(ARCHIVO_FAVORITOS, std::ios::app);
        archivo << comando << std::endl;
        archivo.close();
    }
}


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
            // Crear archivo para comandos favoritos
            if (comandos[0][1] == "crear")
            {
                std::ofstream archivo(comandos[0][2]);
                archivo.close();
                std::cout << "Archivo creado en: " << comandos[0][2] << std::endl;
            }
            // Mostrar lista de comandos favoritos
            else if (comandos[0][1] == "mostrar")
            {
                std::ifstream archivo(ARCHIVO_FAVORITOS);
                std::string linea;
                int i = 1;
                while (std::getline(archivo, linea))
                {
                    std::cout << i << ". " << linea << std::endl;
                    i++;
                }
                archivo.close();
            }
            // Eliminar comandos favoritos por número
            else if (comandos[0][1] == "eliminar")
            {
                std::vector<int> indices_a_eliminar;
                std::istringstream iss(comandos[0][2]);
                std::string numero;
                while (std::getline(iss, numero, ','))
                {
                    indices_a_eliminar.push_back(std::stoi(numero) - 1);
                }

                std::ifstream archivo(ARCHIVO_FAVORITOS);
                std::vector<std::string> nuevos_comandos;
                std::string linea;
                int index = 0;
                while (std::getline(archivo, linea))
                {
                    if (std::find(indices_a_eliminar.begin(), indices_a_eliminar.end(), index) == indices_a_eliminar.end())
                    {
                        nuevos_comandos.push_back(linea);
                    }
                    index++;
                }
                archivo.close();

                std::ofstream archivo2(ARCHIVO_FAVORITOS);
                for (const auto &cmd : nuevos_comandos)
                {
                    archivo2 << cmd << std::endl;
                }
                archivo2.close();
                std::cout << "Comandos eliminados." << std::endl;
            }
            // Buscar comandos favoritos
            else if (comandos[0][1] == "buscar")
            {
                std::ifstream archivo(ARCHIVO_FAVORITOS);
                std::string linea;
                while (std::getline(archivo, linea))
                {
                    if (linea.find(comandos[0][2]) != std::string::npos)
                    {
                        std::cout << linea << std::endl;
                    }
                }
                archivo.close();
            }
            // Borrar todos los comandos favoritos
            else if (comandos[0][1] == "borrar")
            {
                std::ofstream archivo(ARCHIVO_FAVORITOS, std::ios::trunc);
                archivo.close();
                std::cout << "Todos los comandos favoritos han sido borrados." << std::endl;
            }
            // Ejecutar un comando favorito por su número
            else if (comandos[0][1] == "ejecutar")
            {
                std::ifstream archivo(ARCHIVO_FAVORITOS);
                std::string linea;
                std::vector<std::string> comandos_favoritos;
                while (std::getline(archivo, linea))
                {
                    comandos_favoritos.push_back(linea);
                }
                archivo.close();
                std::string comando_a_ejecutar = comandos_favoritos[std::stoi(comandos[0][2]) - 1];
                std::cout << "Ejecutando: " << comando_a_ejecutar << std::endl;
                system(comando_a_ejecutar.c_str());
            }
            // Cargar comandos favoritos desde el archivo
            else if (comandos[0][1] == "cargar")
            {
                std::ifstream archivo(ARCHIVO_FAVORITOS);
                std::string linea;
                while (std::getline(archivo, linea))
                {
                    //Agregar los comandos del archivo de favoritos a el historial de la shell actual
                }
                archivo.close();
            }
            // Guardar el historial de la sesión actual en los favoritos
            else if (comandos[0][1] == "guardar")
            {
                std::ifstream archivo(ARCHIVO_FAVORITOS);
                std::set<std::string> comandos_unicos;
                std::string linea;
                while (std::getline(archivo, linea))
                {
                    comandos_unicos.insert(linea);
                }
                archivo.close();

                std::ofstream archivo_escritura(ARCHIVO_FAVORITOS, std::ios::app);
                HIST_ENTRY **historial = history_list();
                for (int i = 0; historial[i] != NULL; i++)
                {
                    std::string comando = historial[i]->line;
                    if (comandos_unicos.find(comando) == comandos_unicos.end())
                    {
                        archivo_escritura << comando << std::endl;
                        comandos_unicos.insert(comando);
                    }
                }
                archivo_escritura.close();
                std::cout << "Historial guardado en favoritos." << std::endl;
            }
        }
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
            catch (const std::invalid_argument&) 
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

        agregar_a_favoritos(comando);
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
