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
        if(command.compare("REGISTER") == 0 | command.compare("LOGIN") == 0 ){
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
            for(int i=0 ;i<username.length(); i=i+1){
               lineByFormat.push_back(username.at(i));
            }
            lineByFormat.push_back('\0');
            //password
            std::string password = line.substr(line.find_first_of(' ')+1);
            for(int i=0 ;i<password.length(); i=i+1){
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
            line = line.substr(line.find_first_of(' ')+2);
            string numOfUsers(line.substr(0,line.find_first_of(' ')));
            char numOfUsersArr[2];
            shortToBytes(atoi(numOfUsers.c_str()), numOfUsersArr);
            lineByFormat.push_back(numOfUsersArr[0]);
            lineByFormat.push_back(numOfUsersArr[1]);
            //user name list
            line = line.substr(line.find_first_of(' ')+1);
            vector<string> userNameList;
            while(line.find(' ')!=-1){
                userNameList.push_back(line.substr(0, line.find_first_of(' ')));
                line = line.substr(line.find_first_of(' ')+1);
            }
            for(int i =0; i<userNameList.size() ; i= i+1){
                copy(userNameList.at(i).begin(), userNameList.at(i).end(), back_inserter(lineByFormat));
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
        string newLine(lineByFormat.begin(), lineByFormat.end());
        return newLine;

    }
    short bytesToShort(char* bytesArr)
    {
        short result = (short)((bytesArr[0] & 0xff) << 8);
        result += (short)(bytesArr[1] & 0xff);
        return result;
    }

    void shortToBytes(short num, char* bytesArr)
    {
        bytesArr[0] = ((num >> 8) & 0xFF);
        bytesArr[1] = (num & 0xFF);
    }
    std::string decode(std::string line){
        std::vector<char> bytes(line.begin(), line.end());
        std::string answer("");
        bytes.push_back('\0');
        char opCodeB[2] = {bytes[0],bytes[1]};
        short opCode  = bytesToShort(opCodeB);
        if(opCode == 9){
            answer += "NOTIFICATION ";
            if (bytes[2] == '0'){
                answer += "PM ";
            }
            else{
                answer += "Public ";
            }
            int i = 3;
            char t = bytes[i];
            while(t != '\0'){
                answer += t;
                std::cout << t << std::endl;
                i++;
                t =bytes[i];
            }
            i++; //to jump over the zero byte
            t = bytes[i];
            answer += " ";
            while(t != '\0'){
                answer += t;
                std::cout << t << std::endl;
                i++;
                t =bytes[i];
            }

        }
        else if(opCode == 10){
            answer += "ACK ";
            //massage opcode
            char opCodeMB[2] = {bytes[2],bytes[3]};
            short mOpCode  = bytesToShort(opCodeMB);
            answer += mOpCode + " ";
            if(mOpCode == 8){
                char numUserP[2] = {bytes[4],bytes[5]};
                short numUser  = bytesToShort(numUserP);
                answer += numUser;

                char numUserF[2] = {bytes[6],bytes[7]};
                short numfollow  = bytesToShort(numUserF);
                answer += " "+ numfollow;
                char numUserMe[2] = {bytes[8],bytes[9]};
                short numFMe  = bytesToShort(numUserMe);
                answer += " " +numFMe;
            }
            if(mOpCode == 7) {
                char numUserB[2] = {bytes[4],bytes[5]};
                short numUser  = bytesToShort(numUserB);
                answer += " " + numUser;
                answer += " ";
                char t =bytes[6];
                for(int j = 6;j<bytes.size()-1; j++) {
                    if (t != '\0') {
                        answer += t;
                        t = bytes[j];
                    }
                    else answer += " ";
                }
            }
            if(mOpCode == 4){
                char numUserB[2] = {bytes[4],bytes[5]};
                short numUser  = bytesToShort(numUserB);
                answer += numUser;
                answer += " ";
                char t =bytes[6];
                for(int j = 6;j<bytes.size()-1; j++) {
                    if (t != '\0') {
                        answer += t;
                        t = bytes[j];
                    }
                    else answer += " ";
                }
            }

        }
        else if(opCode == 11){
            answer += "Error ";
            char opCodeMB[2] = {bytes[2],bytes[3]};
            short mOpCode  = bytesToShort(opCodeMB);
            answer += mOpCode;
        }
        else{
            std::cout << "not correct message" << std::endl;
        }
        std::cout << answer << std::endl;
        return answer;
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
            int len = answer.length();
            // A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
            // we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
            answer.resize(len - 1);
            std::string toPrint = decode(answer);
            if (answer == "ACK<3>") {
                std::cout << "Exiting...\n" << std::endl;
                break;
            }
            else     std::cout << toPrint << std::endl;

        }
    }
};