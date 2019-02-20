mkdt
=======

Demonstration usage
------
    1. Start the local mkdt server called mkdt_server
        a with not arguments so using the default port
        b a usable tcp port number as argument
    2. Start the mkdt_example_client
        a with not arguments so using the default port
        b the same port number as 1.a.
    3. type "register service example_service_name" and press enter
    4. optional start another mkdt_example_client for the next commands
    5. type "use service example_service_name"
        a. local server should output protocol conversation
        b. example_client should report success

#Building
##Dependencies
To build this project following libraries are needed
*   Boost >= 1.66

##Project
Building is done via cmake.
So after cloning one could for example
```
mkdir bin
cd bin
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
ctest
```
