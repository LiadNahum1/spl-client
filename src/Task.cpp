//
// Created by liad on 12/25/18.
//

#include <iostream>
#include <mutex>
#include <thread>
#include <stdlib.h>
#include "../include/connectionHandler.h"

class Task{
private:
    ConnectionHandler & conn;
    std::mutex & _mutex;
public:
    Task (ConnectionHandler& con, std::mutex& mutex) : conn(con), _mutex(mutex) {}

    void sendToServer() {
        while (1) {
            const short bufsize = 1024;
            char buf[bufsize];
            //we get the line
            std::cin.getline(buf, bufsize);
            //encode
            std::string line(buf);
            encode(line);
            int len = line.length();
            if (!conn.sendLine(line)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            // connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
            std::cout << "Sent " << len + 1 << " bytes to server" << std::endl;

        }
    }

    void encode(std::string line){
        char bytes[line.length()]; //TODO:: what????
        std::string command = line.substr(0,line.find_first_of(' '));
        char opcode [2];


        if(command.compare("REGISTER") == 0){
            //opcode
            shortToBytes(1, opcode);
            bytes[0]= opcode[0];
            bytes[1] = opcode[1];
            //username
            line = line.substr(line.find_first_of(' '));
            std::string username = line.substr(0, line.find_first_of(' '));
            for(int i=0 ;i<username.length(); i=i+1){
                bytes[i+2] = username.at(i);
            }
            bytes[username.length() + 1] = '\0';
            //password
            std::string password = line.substr(line.find_first_of(' '));

        }

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
            answer += "NOTIFICATION";
            if (bytes[2] == '0'){
                answer += "<PM>";
            }
            else{
                answer += "<Public><";
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
            answer += "><";
            while(t != '\0'){
                answer += t;
                std::cout << t << std::endl;
                i++;
                t =bytes[i];
            }
            answer+=">";
        }
        else if(opCode == 10){
            answer += "ACK<";
            char opCodeMB[2] = {bytes[2],bytes[3]};
            short mOpCode  = bytesToShort(opCodeMB);
            answer += mOpCode + ">";
            if(mOpCode == 8){
                char numUserB[2] = {bytes[4],bytes[5]};
                short numUser  = bytesToShort(numUserB);
                answer += numUser;

                char numUserB[2] = {bytes[4],bytes[5]};
                short numUser  = bytesToShort(numUserB);
                answer += numUser;
                char numUserB[2] = {bytes[4],bytes[5]};
                short numUser  = bytesToShort(numUserB);
                answer += numUser;
            }
            int i = 0; // print the userlist
            i++; //to jump over the zero byte
            answer += "><";
            while(t != '\0'){
                answer += t;
                std::cout << t << std::endl;
                i++;
                t =bytes[i];
            }



        }
        else if(opCode == 11){}
        else{
            std::cout << "not correct message" << std::endl;
        }
        return answer;
    }
    void getFromServer(){
        std::string answer;
        // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
        // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
        if (!conn.getLine(answer)) {
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            break;
        }

         int len=answer.length();
        // A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
        // we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
        answer.resize(len-1);
        std::string toPrint = decode(answer);

        std::cout << "Reply: " << answer << " " << len << " bytes " << std::endl << std::endl;
        if (answer == "bye") {
            std::cout << "Exiting...\n" << std::endl;
            break;
        }
    }
};