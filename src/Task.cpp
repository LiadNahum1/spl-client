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
    std::mutex & _mutex;
public:
    Task (ConnectionHandler& con, std::mutex& mutex) : conn(con), _mutex(mutex) {}

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
        while (1) {
            const short bufsize = 1024;
            char buf[bufsize];
            //we get the line
            std::cin.getline(buf, bufsize);
            //encode
            std::string line(buf);
            string newLine = encode(line);
            int len = newLine.length();

            if (!conn.sendLine(newLine)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            if(line.compare("LOGOUT") == 0){
                break;
            }
            // connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
            std::cout << "Sent " << len + 1 << " bytes to server" << std::endl;

        }
    }
    void getFromServer() {
        std::string answer;
        while (true) {
            // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
            // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
            if (!conn.getLine(answer)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }

            if (answer == "ACK3") {
                std::cout << "Exiting...\n" << std::endl;
                break;
            }
            else     std::cout << answer << std::endl;
            answer = "";
        }
    }
};