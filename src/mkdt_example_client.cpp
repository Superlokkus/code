/*! @file mkdt_example_client.cpp
 *
 */

#include <iostream>
#include <string>
#include <map>


#include <mkdt_lib.hpp>
#include <object_remoting_mockup.hpp>


const char *const menu_text{R"END_MENU_TEXT(
Menu:
=====
quit
disconnect
reconnect
----------
register service <name>
use service <name>
----------
create object
destroy object <id>
expose object <id>
consume object <uid>
---------
object <id> example_method_1 <word> <number>
)END_MENU_TEXT"};

void connect() {

}

void disconnect() {

}

int main() {
    std::cout << "MKDT example client\n";

    std::cout << menu_text << "Input: ";
    for (std::string input; std::cin >> input; std::cout << "Input: ") {
        if (input == "quit")
            break;
        if (input == "disonnect")
            disconnect();
        if (input == "reconnect") {
            disconnect();
            connect();
        }
    }

}
