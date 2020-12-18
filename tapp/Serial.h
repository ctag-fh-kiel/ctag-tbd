#pragma once

#include <boost/asio.hpp>

class Serial
{
public:
    Serial(std::string port, unsigned int baud_rate)
    : io(), serial(io,port)
    {
        serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    }

    void writeString(std::string s)
    {
        std::string cmd = stx + s + etx;
        boost::asio::write(serial,boost::asio::buffer(cmd.c_str(),cmd.size()));
    }

    std::string readString()
    {
        using namespace boost;
        char c;
        std::string result = "";
        for(;;)
        {
            asio::read(serial,asio::buffer(&c,1));
            if(pState == IDLE){
                printf("%c", c);
            }
            if(c == stx){
                pState = RCV;
                result = "";
            }else if(c == etx){
                pState = IDLE;
                printf("\n");
                return result;
            }else if(pState == RCV){
                printf("%c", c);
                result += c;
            }
        }
    }

private:
    boost::asio::io_service io;
    boost::asio::serial_port serial;
    const char stx = 2;
    const char etx = 3;
    enum ProtocolStates {
        IDLE = 0x00,
        RCV = 0x01
    };
    ProtocolStates pState = IDLE;
};


