//
// Created by liad on 12/25/18.
//

#include "../include/connectionHandler.h"
#include <iostream>
#include <mutex>

using boost::asio::ip::tcp;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;


ConnectionHandler::ConnectionHandler(string host, short port): host_(host), port_(port), io_service_(), socket_(io_service_){}

ConnectionHandler::~ConnectionHandler() {
    close();
}

bool ConnectionHandler::connect() {
    std::cout << "Starting connect to "
              << host_ << ":" << port_ << std::endl;
    try {
        tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
        boost::system::error_code error;
        socket_.connect(endpoint, error);
        if (error)
            throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {

    size_t tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp ) {
            tmp += socket_.read_some(boost::asio::buffer(bytes+tmp, bytesToRead-tmp), error);
        }
        if(error) {
            cout<<"Error"<<endl;
            throw boost::system::system_error(error);
        }
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {

    int tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp ) {
            tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
        if(error)
            throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

short bytesToShort(char* bytesArr)
{
    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}

bool ConnectionHandler::getLine(std::string& line) {
    char opcode[2];
    getBytes(opcode, 2);
    short opCodeShort = bytesToShort(opcode);

    //Notification
    if(opCodeShort == 9) {
        line.append("NOTIFICATION ");
        char pm[1];
        getBytes(pm, 1);
        if(pm[0] == '0'){ //private message
            line.append("PM ");
        }
        else
            line.append("Public ");
        getFrameAscii(line , '\0'); //posting user
        line.append(" ");
        getFrameAscii(line , '\0');//content

    }
        //ACK OR ERROR
    else if(opCodeShort == 10 || opCodeShort == 11){
        char msgOpCode[2];
        getBytes(msgOpCode, 2);
        short msgOpCodeShort= bytesToShort(msgOpCode);
        if(opCodeShort == 10) {
            line.append("ACK ");
            line.append(std::to_string(msgOpCodeShort));
            if (msgOpCodeShort == 4 || msgOpCodeShort == 7 || msgOpCodeShort == 8) {
                line.append(" ");
                //NumOfUsers or NumPosts
                char number[2];
                getBytes(number, 2);
                short mNum = bytesToShort(number);
                line.append(std::to_string(mNum));
                line.append(" ");
                //FOLLOW OF USERLIST
                if (msgOpCodeShort == 4 || msgOpCodeShort == 7) {
                    //UserNameList
                    for (int i = 0; i < mNum; i++) {
                        getFrameAscii(line, '\0');
                        line.append(" ");
                    }
                }
                    //STAT
                else {
                    char numFollowers[2];
                    getBytes(numFollowers, 2);
                    short followers = bytesToShort(numFollowers);
                    line.append(std::to_string(followers));
                    line.append(" ");
                    char numFollowing[2];
                    getBytes(numFollowing, 2);
                    short following = bytesToShort(numFollowing);
                    line.append(std::to_string(following));
                    line.append(" ");
                }
            }
        }
            //ERROR
        else{
            line.append("ERROR ");
            line.append(std::to_string(msgOpCodeShort));
        }
    }
    return true;
}

bool ConnectionHandler::sendLine(std::string& line) {
    return sendFrameAscii(line, '\n');
}

bool ConnectionHandler::getFrameAscii(std::string& frame, char delimiter) {
    char ch;
    // Stop when we encounter the null character.
    // Notice that the null character is not appended to the frame string.
    try {

        do{
            getBytes(&ch, 1);
            if(ch!= delimiter)
                frame.append(1, ch);
        }while (delimiter != ch);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::sendFrameAscii(const std::string& frame, char delimiter) {
    bool result=sendBytes(frame.c_str(),frame.length());
    if(!result) return false;
    return sendBytes(&delimiter,1);
}

// Close down the connection properly.
void ConnectionHandler::close() {
    try{
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}