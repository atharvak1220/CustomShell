#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main()
{
    while (true)
    {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        cout << cwd << "$ ";

        string command;
        getline(cin, command);

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

        if (words.empty())
            continue;

        //---------------- Built-in: exit ----------------//
        if (words[0] == "exit")
        {
            break;
        }

        //---------------- Built-in: cd ----------------//
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

            continue;
        }

        //---------------- Built-in: pwd ----------------//

        if (words[0]== "pwd")
        {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != nullptr)
            {
                cout << cwd << endl;
            }
            else
            {
                perror("getcwd");
            }
            continue;
        }
        //---------------- Built-in: env ----------------//

        if (words[0]== "env")
        {
            for (char **env = environ; *env != nullptr; ++env)
            {
                cout << *env << endl;
            }
            continue;
        }

        //---------------- Built-in: export ----------------//

        if (words[0]=="export"){
            if(words.size()<2){
                cout << "Usage: export <variable>=<value>\n";
            }
            string var = words[1];
            size_t pos = var.find('=');
            if (pos == string::npos){
                cout << "Usage: export <variable>=<value>\n";
                continue;
            }
            string key = var.substr(0, pos);
            string value = var.substr(pos + 1);
            if (setenv(key.c_str(), value.c_str(), 1) != 0){
                perror("setenv");
            }
        }

        //--------------- Built-in: echo ----------------//

        if(words[0] == "echo")
        {
            for(size_t i = 1; i < words.size(); i++)
            {
                if(words[i][0] == '$')
                {
                    char *value = getenv(words[i].substr(1).c_str());

                    if(value)
                        cout << value;
                }
                else
                {
                    cout << words[i];
                }

                if(i != words.size() - 1)
                    cout << " ";
            }

            cout << endl;

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
