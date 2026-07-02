#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

class Builtin
{
public:
    bool execute(vector<string>& words)
    {
        if(words.empty())
            return true;

        if(words[0] == "pwd")
        {
            char cwd[1024];

            if(getcwd(cwd, sizeof(cwd)) != nullptr)
                cout << cwd << endl;
            else
                perror("getcwd");

            return true;
        }
        if (words[0] == "exit")
        {
            exit(0);
        }
        if(words[0] == "env")
        {
            for(char **env = environ; *env != nullptr; env++)
                cout << *env << endl;

            return true;
        }
        if (words[0] == "cd")
        {
            int result = 0;

            if (words.size() < 2)
            {
                cout << "Usage: cd <directory>\n";
            }
            else if (words.size() == 2)
            {
                result = chdir(words[1].c_str());
            }
            else
            {
                cout << "Usage: cd <directory>\n";
            }

            if (result != 0)
            {
                perror("cd");
            }

            return true;
        }
        if (words[0]=="export"){
            if(words.size()<2){
                cout << "Usage: export <variable>=<value>\n";
                return true;
            }
            string var = words[1];
            size_t pos = var.find('=');
            if (pos == string::npos){
                cout << "Usage: export <variable>=<value>\n";
                return true;
            }
            string key = var.substr(0, pos);
            string value = var.substr(pos + 1);
            if (setenv(key.c_str(), value.c_str(), 1) != 0){
                perror("setenv");
            }
            return true;
        }
        
        // //--------------- Built-in: echo ----------------//

        // if(words[0] == "echo")
        // {
        //     for(size_t i = 1; i < words.size(); i++)
        //     {
        //         if(words[i][0] == '$')
        //         {
        //             char *value = getenv(words[i].substr(1).c_str());

        //             if(value)
        //                 cout << value;
        //         }
        //         else
        //         {
        //             cout << words[i];
        //         }

        //         if(i != words.size() - 1)
        //             cout << " ";
        //     }

        //     cout << endl;

        //     continue;
        // }



        return false;
    }
};

string trim(const string &str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");

    if (start == string::npos)
        return "";

    return str.substr(start, end - start + 1);
}

int main()
{
    Builtin builtins;
    while (true)
    {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        cout << cwd << "$ ";

        string command;
        getline(cin, command);
        
        string inputfile = "";
        string outputfile = "";
        bool append = false;

        size_t pos = command.find(">>");
        if (pos != string::npos)
        {
            outputfile = trim(command.substr(pos + 2));
            command = trim(command.substr(0, pos));
            append = true;
        }
        else
        {
            size_t pos = command.find('>');
            if (pos != string::npos)
            {
                outputfile = trim(command.substr(pos + 1));
                command = trim(command.substr(0, pos));
            }
        }

        size_t inputPos = command.find('<');
        if (inputPos != string::npos)
        {
            inputfile = trim(command.substr(inputPos + 1));
            command = trim(command.substr(0, inputPos));
        }

        if (command.empty())
            continue;

        // Split command into words
        stringstream ss(command);
        vector<string> words;
        string temp;

        while (ss >> temp)
        {
            words.push_back(temp);
        }

        if(words.empty())
            continue;

        if(builtins.execute(words))
        {
            continue;
        }

        //---------------- External Commands ----------------//

        vector<char*> args;

        for (string &s : words)
        {
            // Safe: s.data() is guaranteed to be null-terminated in C++11 and later
            args.push_back(const_cast<char*>(s.data()));
        }

        args.push_back(nullptr);

        pid_t pid = fork();

        if (pid == 0)
        {
            if(!inputfile.empty())
            {
               int fd = open(inputfile.c_str(), O_RDONLY);
               if(fd < 0)
               {
                   perror("open input file");
                   exit(1);
               }
               dup2(fd, STDIN_FILENO);
               close(fd);
            }
            if(!outputfile.empty())
            {
                int fd;
                if(append)
                {
                    fd = open(outputfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                }
                else
                {
                    fd = open(outputfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                if(fd < 0)
                {
                    perror("open output file");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(args[0], args.data());

            perror("Command Failed");
            exit(1);
        }
        else if (pid > 0)
        {
            wait(nullptr);
        }
        else
        {
            perror("fork");
        }
    }

    return 0;
}
