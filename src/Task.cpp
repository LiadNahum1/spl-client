//
// Created by liad on 12/25/18.
//

#include <iostream>
#include <mutex>
#include <thread>
#include <stdlib.h>
#include "../include/connectionHandler.h"
using namespace std;
class Task{
private:
    ConnectionHandler & conn;
    bool & isFailedLogout;
    bool & shouldTerminate;
public:
    Task (ConnectionHandler& con, bool& isfail, bool& shouldTer) : conn(con), isFailedLogout(isfail), shouldTerminate(shouldTer) {}

    string encode(std::string line){
        std::vector<char> lineByFormat; //TODO::ON heap?
        std::string command = line.substr(0,line.find_first_of(' '));
        char opcode [2];
        if((command.compare("REGISTER") == 0) | (command.compare("LOGIN") == 0) ){
            if(command.compare("REGISTER") == 0) {
                //opcode
                shortToBytes(1, opcode);
            }
            else
                shortToBytes(2, opcode);
            lineByFormat.push_back(opcode[0]);
            lineByFormat.push_back(opcode[1]);
            //username
            line = line.substr(line.find_first_of(' ')+1);
            std::string username = line.substr(0, line.find_first_of(' '));
            for(int i=0 ;i<(int)username.length(); i=i+1){
                lineByFormat.push_back(username.at(i));
            }
            lineByFormat.push_back('\0');
            //password
            std::string password = line.substr(line.find_first_of(' ')+1);
            for(int i=0 ;i<(int)password.length(); i=i+1){
                lineByFormat.push_back(password.at(i));
            }
            lineByFormat.push_back('\0');
        }

        else if(command.compare("LOGOUT") == 0){
            //opcode
            shortToBytes(3, opcode);
            lineByFormat.push_back(opcode[0]);
            lineByFormat.push_back(opcode[1]);

        }
        else if(command.compare("FOLLOW") == 0){
            //opcode
            shortToBytes(4, opcode);
            lineByFormat.push_back(opcode[0]);
            lineByFormat.push_back(opcode[1]);
            //follow/unfollow
            std::string isFollow = line.substr(line.find_first_of(' ')+1, 1);
            lineByFormat.push_back(isFollow[0]);
            line = line.substr(line.find_first_of(' ')+3);

            char numOfUsersArr[2];
            //wants to follow after 0 users
            if(line.find(' ') == -1){
                shortToBytes(0, numOfUsersArr);
                lineByFormat.push_back(numOfUsersArr[0]);
                lineByFormat.push_back(numOfUsersArr[1]);
            }
            else {
                string numOfUsers(line.substr(0, line.find_first_of(' ')));
                shortToBytes(atoi(numOfUsers.c_str()), numOfUsersArr);
                lineByFormat.push_back(numOfUsersArr[0]);
                lineByFormat.push_back(numOfUsersArr[1]);
                //user name list
                line = line.substr(line.find_first_of(' ') + 1);
                string user;
                //more than one user to follow after there is a space
                while ((int) line.find(' ') != -1) {
                    user = line.substr(0, line.find_first_of(' '));
                    copy(user.begin(), user.end(), back_inserter(lineByFormat));
                    lineByFormat.push_back('\0');
                    line = line.substr(line.find_first_of(' ') + 1);
                }
                //there is one user to follow after
                if (line != "") {
                    copy(line.begin(), line.end(), back_inserter(lineByFormat));
                }
            }
            lineByFormat.push_back('\0');


        }
        else if(command.compare("POST") == 0){
            //opcode
            shortToBytes(5, opcode);
            lineByFormat.push_back(opcode[0]);
            lineByFormat.push_back(opcode[1]);
            //content
            string content = line.substr(line.find_first_of(' ') +1);
            copy(content.begin(), content.end(), back_inserter(lineByFormat));
            lineByFormat.push_back('\0');

        }
        else if(command.compare("PM") == 0){
            shortToBytes(6, opcode);
            lineByFormat.push_back(opcode[0]);
            lineByFormat.push_back(opcode[1]);
            //username
            line = line.substr(line.find_first_of(' ') + 1);
            string username = line.substr(0, line.find_first_of(' '));
            copy(username.begin(), username.end(), back_inserter(lineByFormat));
            lineByFormat.push_back('\0');
            //content
            string content = line.substr(line.find_last_of(' ') + 1);
            copy(content.begin(), content.end(), back_inserter(lineByFormat));
            lineByFormat.push_back('\0');
        }
        else if(command.compare("USERLIST") == 0){
            shortToBytes(7, opcode);
            lineByFormat.push_back(opcode[0]);
            lineByFormat.push_back(opcode[1]);
        }
        else if(command.compare("STAT") == 0){
            shortToBytes(8, opcode);
            lineByFormat.push_back(opcode[0]);
            lineByFormat.push_back(opcode[1]);
            //username
            string username = line.substr(line.find_first_of(' ') + 1);
            copy(username.begin(), username.end(), back_inserter(lineByFormat));
            lineByFormat.push_back('\0');
        }
        else{
            std::cout << "illegal command" << std::endl;
            return "";
        }
        string newLine(lineByFormat.begin(), lineByFormat.end());
        return newLine;

    }

    void shortToBytes(short num, char* bytesArr)
    {
        bytesArr[0] = ((num >> 8) & 0xFF);
        bytesArr[1] = (num & 0xFF);
    }

    void sendToServer() {
        while (!shouldTerminate) {
            const short bufsize = 1024;
            char buf[bufsize];
            std::cin.getline(buf, bufsize);
            //encode
            std::string line(buf);
            string newLine = encode(line);
            if(newLine != "") {
                if(!shouldTerminate) {
                    if (!conn.sendLine(newLine)) {
                        std::cout << "Disconnected. Exiting...\n" << std::endl;
                        shouldTerminate = true;
                    }
                    if(line == "LOGOUT"){
                        while(1){
                         if(shouldTerminate)
                             return;
                         else
                         {
                             if(isFailedLogout) {
                                 isFailedLogout = false;
                                 break;
                             }
                         }
                        }
                    }
                }
            }

        }
    }
    void getFromServer() {
        std::string answer;
        while (!shouldTerminate) {
            if (!conn.getLine(answer)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                shouldTerminate = true;
            }
            //SO I WILL KNOW A LOGOUT ATEMPTION FAILED
            if(answer == "ERROR 3"){
                isFailedLogout = true;
            }
            if (answer == "ACK 3") {
                std::cout << answer << std::endl;
                shouldTerminate = true;
            }

            else     std::cout << answer << std::endl;
            answer = "";
        }
    }
};